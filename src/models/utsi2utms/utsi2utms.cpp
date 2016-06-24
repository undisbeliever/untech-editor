#include "utsi2utms.h"
#include "tilesetinserter.h"
#include "models/metasprite-common/framesetexportorder.h"
#include "models/metasprite.h"
#include "models/sprite-importer.h"
#include <algorithm>
#include <cassert>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>

#include <iostream>

const size_t PALETTE_COLORS = 16;

namespace UnTech {
namespace Utsi2UtmsPrivate {

namespace MS = UnTech::MetaSprite;
namespace SI = UnTech::SpriteImporter;

inline Snes::Tile4bpp8px getSmallTile(const Image& image,
                                      const std::map<rgba, unsigned>& colorMap,
                                      const SI::FrameObject& siObj)
{
    unsigned xOffset = siObj.frame().location().x + siObj.location().x;
    unsigned yOffset = siObj.frame().location().y + siObj.location().y;

    Snes::Tile4bpp8px tile;
    uint8_t* tData = tile.rawData();
    for (unsigned y = 0; y < 8; y++) {
        const rgba* imgBits = image.scanline(yOffset + y) + xOffset;

        for (unsigned x = 0; x < 8; x++) {
            *tData++ = colorMap.at(*imgBits++);
        }
    }

    return tile;
}

inline Snes::Tile4bpp16px getLargeTile(const Image& image,
                                       const std::map<rgba, unsigned>& colorMap,
                                       const SI::FrameObject& siObj)
{
    unsigned xOffset = siObj.frame().location().x + siObj.location().x;
    unsigned yOffset = siObj.frame().location().y + siObj.location().y;

    Snes::Tile4bpp16px tile;
    uint8_t* tData = tile.rawData();

    for (unsigned y = 0; y < 16; y++) {
        const rgba* imgBits = image.scanline(yOffset + y) + xOffset;

        for (unsigned x = 0; x < 16; x++) {
            *tData++ = colorMap.at(*imgBits++);
        }
    }

    return tile;
}

// mark the pixels in the undertile that are overlapped by the overtile
template <unsigned OVER_SIZE, unsigned UNDER_SIZE>
inline std::array<bool, UNDER_SIZE * UNDER_SIZE>
markOverlappedPixels(const Snes::Tile<4, OVER_SIZE>& overTile,
                     int xOffset, int yOffset)
{
    const uint8_t* overTileData = overTile.rawData();

    std::array<bool, UNDER_SIZE* UNDER_SIZE> ret = {};

    for (unsigned oY = 0; oY < OVER_SIZE; oY++) {
        int uY = oY + yOffset;

        if (uY >= 0 && uY < (int)UNDER_SIZE) {
            for (unsigned oX = 0; oX < OVER_SIZE; oX++) {
                int uX = oX + xOffset;

                if (uX >= 0 && uX < (int)UNDER_SIZE) {
                    if (overTileData[oY * OVER_SIZE + oX] != 0) {
                        ret[uY * UNDER_SIZE + uX] = true;
                    }
                }
            }
        }
    }

    return ret;
}

// clears the pixels in the overtile that match the undertile.
template <unsigned OVER_SIZE, unsigned UNDER_SIZE>
inline void clearCommonOverlappedTiles(Snes::Tile<4, OVER_SIZE>& overTile,
                                       Snes::Tile<4, UNDER_SIZE>& underTile,
                                       int xOffset, int yOffset)
{
    uint8_t* overTileData = overTile.rawData();
    uint8_t* underTileData = underTile.rawData();

    for (unsigned oY = 0; oY < OVER_SIZE; oY++) {
        int uY = oY + yOffset;

        if (uY >= 0 && uY < (int)UNDER_SIZE) {
            for (unsigned oX = 0; oX < OVER_SIZE; oX++) {
                int uX = oX + xOffset;

                if (uX >= 0 && uX < (int)UNDER_SIZE) {
                    if (overTileData[oY * OVER_SIZE + oX] == underTileData[uY * UNDER_SIZE + uX]) {
                        overTileData[oY * OVER_SIZE + oX] = 0;
                    }
                }
            }
        }
    }
}
}
}

using namespace UnTech;
using namespace UnTech::Utsi2UtmsPrivate;

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
    msFrameSet.setTilesetType(siFrameSet.tilesetType());

    if (siFrameSet.exportOrderDocument()) {
        msFrameSet.loadExportOrderDocument(siFrameSet.exportOrderDocument()->filename());
    }

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

    auto getTilesetOutputFromImage = [&](const SI::FrameObject& siObj) {
        if (siObj.size() == SI::FrameObject::ObjectSize::SMALL) {
            return smallTileset.getOrInsert(getSmallTile(image, colorMap, siObj));
        }
        else {
            return largeTileset.getOrInsert(getLargeTile(image, colorMap, siObj));
        }
    };

    // A Mapping to store all the frame objects that overlap each other.
    // Mapping of <frameName> -> <objectIDs> -> list<overlapping objectIDs>
    std::map<const std::string, std::map<unsigned, std::list<unsigned>>> overlappingFrameObjectsMap;

    // Process frames
    for (const auto frameIt : siFrameSet.frames()) {
        const SI::Frame& siFrame = frameIt.second;
        const auto& siFrameOrigin = siFrame.origin();

        MS::Frame* msFramePtr = msFrameSet.frames().create(frameIt.first);
        MS::Frame& msFrame = *msFramePtr;

        std::unordered_set<const SI::FrameObject*> overlapping;

        // Search for overlapping frame objects
        {
            typedef SI::FrameObject::list_t::const_iterator f_iterator;

            const auto& fobjs = siFrame.objects();

            for (f_iterator iIt = fobjs.begin(); iIt != fobjs.end(); ++iIt) {
                const SI::FrameObject& iObj = *iIt;
                const urect iRect(iObj.location(), iObj.sizePx());

                for (f_iterator jIt = iIt + 1; jIt != fobjs.end(); ++jIt) {
                    const SI::FrameObject& jObj = *jIt;

                    if (iRect.overlaps(jObj.location(), jObj.sizePx())) {
                        overlapping.insert(&iObj);
                        overlapping.insert(&jObj);

                        unsigned di = std::distance(fobjs.begin(), iIt);
                        unsigned dj = std::distance(fobjs.begin(), jIt);
                        overlappingFrameObjectsMap[frameIt.first][di].push_back(dj);
                    }
                }
            }
        }

        try {
            for (const SI::FrameObject& siObj : siFrame.objects()) {
                MS::FrameObject& msObj = msFrame.objects().create();

                msObj.setSize(static_cast<MS::FrameObject::ObjectSize>(siObj.size()));
                msObj.setLocation(ms8point::createFromOffset(siObj.location(), siFrameOrigin));

                if (overlapping.count(&siObj) > 0) {
                    // don't process overlapping tiles here
                    continue;
                }

                auto to = getTilesetOutputFromImage(siObj);
                to.apply(msObj);
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

    bool useSmall;
    struct {
        Snes::Tile4bpp8px small;
        Snes::Tile4bpp16px large;
    } overTile;

    for (const auto overlappingFramesIt : overlappingFrameObjectsMap) {
        const auto& frameObjectOverlaps = overlappingFramesIt.second;

        const SI::Frame& siFrame = siFrameSet.frames().at(overlappingFramesIt.first);
        MS::Frame& msFrame = msFrameSet.frames().at(overlappingFramesIt.first);

        // Check that there is no triple overlapping tile
        {
            std::set<unsigned> matches;
            for (auto foIt : frameObjectOverlaps) {
                for (unsigned id : foIt.second) {
                    auto ret = matches.insert(id);

                    if (ret.second == false) {
                        addError(siFrame, "Cannot have three or more overlapping tiles");
                        return nullptr;
                    }
                }
            }
        }

        std::set<MS::FrameObject*> emptyObjects;

        // Process the overlapping tiles
        for (auto foIt : frameObjectOverlaps) {
            const SI::FrameObject& siOverObj = siFrame.objects().at(foIt.first);
            MS::FrameObject& msOverObj = msFrame.objects().at(foIt.first);

            if (siOverObj.size() == SI::FrameObject::ObjectSize::SMALL) {
                overTile.small = getSmallTile(image, colorMap, siOverObj);
                useSmall = true;
            }
            else {
                overTile.large = getLargeTile(image, colorMap, siOverObj);
                useSmall = false;
            }

            for (unsigned id : foIt.second) {
                const SI::FrameObject& siUnderObj = siFrame.objects().at(id);
                MS::FrameObject& msUnderObj = msFrame.objects().at(id);

                int xOffset = siOverObj.location().x - siUnderObj.location().x;
                int yOffset = siOverObj.location().y - siUnderObj.location().y;

                /*
                 * This:
                 *   - Gets the undertile pixels
                 *   - Mark the undertile pixels that are overlapped with 0xFF
                 *   - Search for duplicate tiles, creating a new tile if necessary
                 */
                std::pair<TilesetInserterOutput, bool> tilesetOutput;
                if (useSmall) {
                    if (siUnderObj.size() == SI::FrameObject::ObjectSize::SMALL) {
                        auto underTile = getSmallTile(image, colorMap, siUnderObj);
                        auto overlaps = markOverlappedPixels<8, 8>(overTile.small, xOffset, yOffset);
                        tilesetOutput = smallTileset.processOverlappedTile(underTile, overlaps);
                    }
                    else {
                        auto underTile = getLargeTile(image, colorMap, siUnderObj);
                        auto overlaps = markOverlappedPixels<8, 16>(overTile.small, xOffset, yOffset);
                        tilesetOutput = largeTileset.processOverlappedTile(underTile, overlaps);
                    }
                }
                else {
                    if (siUnderObj.size() == SI::FrameObject::ObjectSize::SMALL) {
                        auto underTile = getSmallTile(image, colorMap, siUnderObj);
                        auto overlaps = markOverlappedPixels<16, 8>(overTile.large, xOffset, yOffset);
                        tilesetOutput = smallTileset.processOverlappedTile(underTile, overlaps);
                    }
                    else {
                        auto underTile = getLargeTile(image, colorMap, siUnderObj);
                        auto overlaps = markOverlappedPixels<16, 16>(overTile.large, xOffset, yOffset);
                        tilesetOutput = largeTileset.processOverlappedTile(underTile, overlaps);
                    }
                }
                tilesetOutput.first.apply(msUnderObj);

                if (tilesetOutput.second == false) {
                    addWarning(siUnderObj, "Matching undertile not found");
                }

                // remove duplicate pixels in the overtile that match the processed undertile.
                if (useSmall) {
                    if (siUnderObj.size() == SI::FrameObject::ObjectSize::SMALL) {
                        auto underTile = smallTileset.getTile(tilesetOutput.first);
                        clearCommonOverlappedTiles<8, 8>(overTile.small, underTile, xOffset, yOffset);
                    }
                    else {
                        auto underTile = largeTileset.getTile(tilesetOutput.first);
                        clearCommonOverlappedTiles<8, 16>(overTile.small, underTile, xOffset, yOffset);
                    }
                }
                else {
                    if (siUnderObj.size() == SI::FrameObject::ObjectSize::SMALL) {
                        auto underTile = smallTileset.getTile(tilesetOutput.first);
                        clearCommonOverlappedTiles<16, 8>(overTile.large, underTile, xOffset, yOffset);
                    }
                    else {
                        auto underTile = largeTileset.getTile(tilesetOutput.first);
                        clearCommonOverlappedTiles<16, 16>(overTile.large, underTile, xOffset, yOffset);
                    }
                }
            }

            // create the overtile and add to the tileset
            if (useSmall) {
                static const Snes::Tile4bpp8px smallZero{};

                if (overTile.small != smallZero) {
                    auto to = smallTileset.getOrInsert(overTile.small);
                    to.apply(msOverObj);
                }
                else {
                    addWarning(siOverObj, "Overtile is empty - skipping");
                    emptyObjects.insert(&msOverObj);
                }
            }
            else {
                static const Snes::Tile4bpp16px largeZero{};

                if (overTile.large != largeZero) {
                    auto to = largeTileset.getOrInsert(overTile.large);
                    to.apply(msOverObj);
                }
                else {
                    addWarning(siOverObj, "Overtile is empty - skipping");
                    emptyObjects.insert(&msOverObj);
                }
            }
        }

        // remove empty frame objects
        for (MS::FrameObject* obj : emptyObjects) {
            msFrame.objects().remove(obj);
        }
    }

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

void Utsi2Utms::addWarning(const SI::FrameObject& frameObj, const std::string& message)
{
    std::stringstream out;

    const SI::Frame& frame = frameObj.frame();
    const SI::FrameSet& fs = frame.frameSet();

    out << fs.name() << "." << fs.frames().getName(frame).first
        << ":object-" << frame.objects().indexOf(frameObj)
        << ": " << message;

    _warnings.push_back(out.str());
}
