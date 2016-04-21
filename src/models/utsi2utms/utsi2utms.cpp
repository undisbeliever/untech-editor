#include "utsi2utms.h"
#include "tilesetinserter.h"
#include "models/metasprite.h"
#include "models/sprite-importer.h"
#include <cassert>
#include <map>
#include <set>
#include <sstream>

const size_t PALETTE_COLORS = 16;

using namespace UnTech;
using namespace UnTech::Utsi2UtmsPrivate;

namespace MS = UnTech::MetaSprite;
namespace SI = UnTech::SpriteImporter;

Utsi2Utms::Utsi2Utms()
    : _errors()
    , _warnings()
    , _hasError(false)
{
}

std::unique_ptr<MS::MetaSpriteDocument> Utsi2Utms::convert(SI::SpriteImporterDocument& siDocument)
{
    _hasError = false;

    const SI::FrameSet& siFrameSet = siDocument.frameSet();
    const UnTech::Image& image = siDocument.frameSet().image();

    // Validate siFrameSet
    {
        if (image.empty()) {
            addError(siFrameSet, "No Image");
        }
        if (siFrameSet.frames().size() == 0) {
            addError(siFrameSet, "No Frames");
        }
        if (siFrameSet.transparentColorValid() == false) {
            addError(siFrameSet, "Transparent color is invalid");
        }
    }

    if (_hasError) {
        return nullptr;
    }

    auto msDocument = std::make_unique<MS::MetaSpriteDocument>();
    MS::FrameSet& msFrameSet = msDocument->frameSet();

    msFrameSet.setName(siFrameSet.name());

    // Build map of rgba to palette color
    // Faster than std::unordered_map, only contains 16 elements
    std::map<rgba, unsigned> colorMap;
    {
        std::set<rgba> colors;

        for (const auto siFrameIt : siFrameSet.frames()) {
            const SI::Frame& siFrame = siFrameIt.second;

            if (!image.size().contains(siFrame.location())) {
                addError(siFrame, "Frame not inside image");
                continue;
            }

            for (const SI::FrameObject& obj : siFrame.objects()) {
                unsigned lx = siFrame.location().x + obj.location().x;
                unsigned ly = siFrame.location().y + obj.location().y;

                for (unsigned y = 0; y < obj.sizePx(); y++) {
                    const rgba* p = image.scanline(ly + y) + lx;

                    for (unsigned x = 0; x < obj.sizePx(); x++) {
                        colors.insert(*p++);
                    }
                }

                if (colors.size() > PALETTE_COLORS) {
                    addError(siFrameSet, "Too many colors, expected a max of 16");
                    return nullptr;
                }
            }
        }

        auto tIt = colors.find(siFrameSet.transparentColor());
        if (tIt != colors.end()) {
            colors.erase(tIt);
        }
        else {
            addWarning(siFrameSet, "Transparent color is not in frame objects");
        }

        // Verify enough colors after remove transparency
        if (colors.size() > (PALETTE_COLORS - 1)) {
            addError(siFrameSet, "Too many colors, expected a max of 16");
            return nullptr;
        }

        // Store palette in MetaSprite
        // ::TODO handle user supplied palettes::
        {
            MS::Palette& palette = msFrameSet.palettes().create();

            colorMap.insert({ siFrameSet.transparentColor(), 0 });
            palette.color(0).setRgb(siFrameSet.transparentColor());

            int i = 1;
            for (auto& c : colors) {
                colorMap.insert({ c, i });
                palette.color(i).setRgb(c);
                i++;
            }
        }
    }

    if (_hasError) {
        return nullptr;
    }

    TilesetInserter<Snes::Tileset4bpp8px> smallTileset(msFrameSet.smallTileset());
    TilesetInserter<Snes::Tileset4bpp16px> largeTileset(msFrameSet.largeTileset());

    // Process frames
    for (const auto frameIt : siFrameSet.frames()) {
        const SI::Frame& siFrame = frameIt.second;
        const auto& siFrameOrigin = siFrame.origin();
        const auto& siFrameLocation = siFrame.location();

        MS::Frame* msFramePtr = msFrameSet.frames().create(frameIt.first);
        MS::Frame& msFrame = *msFramePtr;

        try {
            for (const SI::FrameObject& siObj : siFrame.objects()) {
                MS::FrameObject& msObj = msFrame.objects().create();

                // ::TODO find overlapping tiles and store in list::
                // ::TODO warning if large tile is in front of and covers a small tile::

                unsigned xOffset = siFrameLocation.x + siObj.location().x;
                unsigned yOffset = siFrameLocation.y + siObj.location().y;

                TilesetInserterOutput tilesetOutput;

                if (siObj.size() == SI::FrameObject::ObjectSize::SMALL) {
                    msObj.setSize(MS::FrameObject::ObjectSize::SMALL);

                    // Get image data
                    std::array<uint8_t, 8 * 8> tile;
                    uint8_t* tData = tile.data();
                    for (unsigned y = 0; y < 8; y++) {
                        const rgba* imgBits = image.scanline(yOffset + y) + xOffset;

                        for (unsigned x = 0; x < 8; x++) {
                            *tData++ = colorMap[*imgBits++];
                        }
                    }

                    tilesetOutput = smallTileset.getOrInsert(tile);
                }
                else {
                    msObj.setSize(MS::FrameObject::ObjectSize::LARGE);

                    // Get image data
                    std::array<uint8_t, 16 * 16> tile;
                    uint8_t* tData = tile.data();
                    for (unsigned y = 0; y < 16; y++) {
                        const rgba* imgBits = image.scanline(yOffset + y) + xOffset;

                        for (unsigned x = 0; x < 16; x++) {
                            *tData++ = colorMap[*imgBits++];
                        }
                    }

                    tilesetOutput = largeTileset.getOrInsert(tile);
                }

                msObj.setLocation(ms8point::createFromOffset(siObj.location(), siFrameOrigin));
                msObj.setTileId(tilesetOutput.tileId);
                msObj.setHFlip(tilesetOutput.hFlip);
                msObj.setVFlip(tilesetOutput.vFlip);
            }

            for (const SI::ActionPoint& siAp : siFrame.actionPoints()) {
                MS::ActionPoint& msAp = msFrame.actionPoints().create();

                msAp.setLocation(ms8point::createFromOffset(siAp.location(), siFrameOrigin));
                msAp.setParameter(siAp.parameter());
            }

            for (const SI::EntityHitbox& siEh : siFrame.entityHitboxes()) {
                MS::EntityHitbox& msEh = msFrame.entityHitboxes().create();

                msEh.setAabb(ms8rect::createFromOffset(siEh.aabb(), siFrameOrigin));
                msEh.setParameter(siEh.parameter());
            }

            if (siFrame.solid()) {
                msFrame.setSolid(true);
                msFrame.setTileHitbox(ms8rect::createFromOffset(siFrame.tileHitbox(), siFrameOrigin));
            }
            else {
                msFrame.setSolid(false);
            }
        }
        catch (const std::out_of_range& ex) {
            // This should not happen unless the frame is very large,
            // a simple error message will do.
            addError(siFrame, ex.what());
            continue;
        }
    }

    if (_hasError) {
        return nullptr;
    }

    // ::TODO process overlapping tiles::

    return msDocument;
}

void Utsi2Utms::addError(const std::string& message)
{
    _errors.push_back(message);
    _hasError = true;
}

void Utsi2Utms::addError(const SI::FrameSet& frameSet, const std::string& message)
{
    std::stringstream out;

    out << frameSet.name()
        << ": "
        << message;

    _errors.push_back(out.str());
    _hasError = true;
}

void Utsi2Utms::addError(const SI::Frame& frame, const std::string& message)
{
    std::stringstream out;

    const SI::FrameSet& fs = frame.frameSet();

    out << fs.name() << "." << fs.frames().getName(frame).first
        << ": " << message;

    _errors.push_back(out.str());
    _hasError = true;
}

void Utsi2Utms::addWarning(const std::string& message)
{
    _warnings.push_back(message);
}

void Utsi2Utms::addWarning(const SI::FrameSet& frameSet, const std::string& message)
{
    std::stringstream out;

    out << frameSet.name() << ": " << message;

    _warnings.push_back(out.str());
}

void Utsi2Utms::addWarning(const SI::Frame& frame, const std::string& message)
{
    std::stringstream out;

    const SI::FrameSet& fs = frame.frameSet();

    out << fs.name() << "." << fs.frames().getName(frame).first
        << ": " << message;

    _warnings.push_back(out.str());
}
