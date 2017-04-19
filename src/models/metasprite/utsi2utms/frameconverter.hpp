#pragma once
#include "tileextractor.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <map>

namespace UnTech {
namespace MetaSprite {
namespace Utsi2UtmsPrivate {

namespace MS = UnTech::MetaSprite::MetaSprite;
namespace SI = UnTech::MetaSprite::SpriteImporter;

class FrameConverter {
    TileExtractor& tileExtractor;

    const SpriteImporter::Frame& siFrame;
    MetaSprite::Frame& msFrame;

    // a mapping of frame objects that over lap each other
    // <over object ID> -> list<under object IDs>
    std::map<unsigned, std::list<unsigned>> overlappingObjects;

public:
    FrameConverter(TileExtractor& tileExtractor,
                   const SpriteImporter::Frame& siFrame,
                   MetaSprite::Frame& msFrame)
        : tileExtractor(tileExtractor)
        , siFrame(siFrame)
        , msFrame(msFrame)
        , overlappingObjects()
    {
        buildOverlappingObjects();
    }

    inline static void applyTilesetOutput(const Snes::TilesetInserterOutput& tio, MS::FrameObject& obj)
    {
        obj.tileId = tio.tileId;
        obj.hFlip = tio.hFlip;
        obj.vFlip = tio.vFlip;
    }

    // processes everything EXCEPT overlapping tiles
    void process()
    {
        auto sieve = buildOverlappingObjectsSieve();

        const auto& siFrameOrigin = siFrame.location.origin;

        for (unsigned objId = 0; objId < siFrame.objects.size(); objId++) {
            const SI::FrameObject& siObj = siFrame.objects.at(objId);
            MS::FrameObject msObj;

            msObj.size = siObj.size;
            msObj.location = ms8point::createFromOffset(siObj.location, siFrameOrigin);

            if (sieve[objId] == false) {
                auto to = tileExtractor.getTilesetOutputFromImage(siFrame, siObj);
                applyTilesetOutput(to, msObj);
            }
            else {
                // don't process overlapping tiles here
                msObj.tileId = UINT_MAX;
            }

            msFrame.objects.emplace_back(msObj);
        }

        for (const SI::ActionPoint& siAp : siFrame.actionPoints) {
            MS::ActionPoint msAp;

            msAp.location = ms8point::createFromOffset(siAp.location, siFrameOrigin);
            msAp.parameter = siAp.parameter;

            msFrame.actionPoints.push_back(msAp);
        }

        for (const SI::EntityHitbox& siEh : siFrame.entityHitboxes) {
            MS::EntityHitbox msEh;

            msEh.aabb = ms8rect::createFromOffset(siEh.aabb, siFrameOrigin);
            msEh.hitboxType = siEh.hitboxType;

            msFrame.entityHitboxes.push_back(msEh);
        }

        msFrame.spriteOrder = siFrame.spriteOrder;

        msFrame.solid = siFrame.solid;
        if (siFrame.solid) {
            msFrame.tileHitbox = ms8rect::createFromOffset(siFrame.tileHitbox, siFrameOrigin);
        }
    }

    // To be called after all frames have been processed
    void processOverlappingTiles()
    {
        for (const auto& foIt : overlappingObjects) {
            const SI::FrameObject& siOverObj = siFrame.objects.at(foIt.first);

            if (siOverObj.size == ObjectSize::SMALL) {
                _processOverlappingTiles<8>(foIt.first, foIt.second);
            }
            else {
                _processOverlappingTiles<16>(foIt.first, foIt.second);
            }
        }

        removeEmptyFrameObjects();
    }

private:
    template <size_t OVER_SIZE>
    inline void _processOverlappingTiles(const unsigned overObjId,
                                         const std::list<unsigned>& underObjectIdList)
    {
        Snes::Tile<OVER_SIZE> overTile;

        const SI::FrameObject& siOverObj = siFrame.objects.at(overObjId);
        MS::FrameObject& msOverObj = msFrame.objects.at(overObjId);

        overTile = tileExtractor.getTileFromImage<OVER_SIZE>(siFrame, siOverObj);

        for (const unsigned underObjId : underObjectIdList) {
            const SI::FrameObject& siUnderObj = siFrame.objects.at(underObjId);
            MS::FrameObject& msUnderObj = msFrame.objects.at(underObjId);

            int xOffset = siOverObj.location.x - siUnderObj.location.x;
            int yOffset = siOverObj.location.y - siUnderObj.location.y;

            /*
             * This:
             *   - Gets the undertile pixels
             *   - Mark the undertile pixels that are overlapped with 0xFF
             *   - Search for duplicate tiles, creating a new tile if necessary
             */
            std::pair<Snes::TilesetInserterOutput, bool> tilesetOutput;
            if (siUnderObj.size == ObjectSize::SMALL) {
                auto underTile = tileExtractor.getTileFromImage<8>(siFrame, siUnderObj);
                auto overlaps = markOverlappedPixels<OVER_SIZE, 8>(overTile, xOffset, yOffset);
                tilesetOutput = tileExtractor.smallTileset.processOverlappedTile(underTile, overlaps);
            }
            else {
                auto underTile = tileExtractor.getTileFromImage<16>(siFrame, siUnderObj);
                auto overlaps = markOverlappedPixels<OVER_SIZE, 16>(overTile, xOffset, yOffset);
                tilesetOutput = tileExtractor.largeTileset.processOverlappedTile(underTile, overlaps);
            }
            applyTilesetOutput(tilesetOutput.first, msUnderObj);

            if (tilesetOutput.second == false) {
                addWarningObj(underObjId, "Matching undertile not found");
            }

            // remove duplicate pixels in the overtile that match the processed undertile.
            if (siUnderObj.size == ObjectSize::SMALL) {
                auto underTile = tileExtractor.smallTileset.getTile(tilesetOutput.first);
                clearCommonOverlappedTiles(overTile, underTile, xOffset, yOffset);
            }
            else {
                auto underTile = tileExtractor.largeTileset.getTile(tilesetOutput.first);
                clearCommonOverlappedTiles(overTile, underTile, xOffset, yOffset);
            }
        }

        // create the overtile and add to the tileset
        static const Snes::Tile<OVER_SIZE> smallZero{};

        if (overTile != smallZero) {
            Snes::TilesetInserterOutput to = tileExtractor.getOrInsertTile(overTile);
            applyTilesetOutput(to, msOverObj);
        }
        else {
            addWarningObj(overObjId, "Overtile is empty - skipping");
        }
    }

    void removeEmptyFrameObjects()
    {
        msFrame.objects.erase(
            std::remove_if(
                msFrame.objects.begin(), msFrame.objects.end(),
                [this](const MS::FrameObject& obj) {
                    return obj.tileId == UINT_MAX;
                }),
            msFrame.objects.end());
    }

    void buildOverlappingObjects()
    {
        typedef std::vector<SI::FrameObject>::const_iterator f_iterator;

        const auto& fobjs = siFrame.objects;

        for (f_iterator iIt = fobjs.begin(); iIt != fobjs.end(); ++iIt) {
            const SI::FrameObject& iObj = *iIt;
            const urect iRect(iObj.location, iObj.sizePx());

            for (f_iterator jIt = iIt + 1; jIt != fobjs.end(); ++jIt) {
                const SI::FrameObject& jObj = *jIt;

                if (iRect.overlaps(jObj.location, jObj.sizePx())) {
                    unsigned di = std::distance(fobjs.begin(), iIt);
                    unsigned dj = std::distance(fobjs.begin(), jIt);

                    overlappingObjects[di].push_back(dj);
                }
            }
        }
    }

    // Also checks that there is no triple overlapping tiles
    std::array<bool, MAX_FRAME_OBJECTS> buildOverlappingObjectsSieve()
    {
        assert(siFrame.objects.size() < MAX_FRAME_OBJECTS);

        std::array<bool, MAX_FRAME_OBJECTS> sieve = {};

        for (const auto& foIt : overlappingObjects) {
            sieve[foIt.first] = 1;

            for (unsigned id : foIt.second) {
                if (sieve[id]) {
                    addError("Cannot have three or more overlapping tiles");
                }
                sieve[id] = 1;
            }
        }

        return sieve;
    }

    inline void addError(const char* message)
    {
        tileExtractor.errorList.addError(tileExtractor.siFrameSet, siFrame, message);
    }

    inline void addWarningObj(unsigned oId, const char* message)
    {
        tileExtractor.errorList.addWarningObj(tileExtractor.siFrameSet, siFrame, oId, message);
    }
};
}
}
}
