/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "tileextractor.hpp"
#include "models/common/iterators.h"
#include "models/metasprite/errorlisthelpers.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <vector>

namespace UnTech::MetaSprite::Utsi2UtmsPrivate {

namespace MS = UnTech::MetaSprite::MetaSprite;
namespace SI = UnTech::MetaSprite::SpriteImporter;

struct OverlappingObject {
    const unsigned overObjectId;
    const std::vector<unsigned> underObjectIds;
};

struct OverlappingObjectFrame {
    const unsigned frameIndex;
    const std::vector<OverlappingObject> overlappingObjects;
};

static void applyTilesetOutput(const Snes::TilesetInserterOutput& tio, MS::FrameObject& obj)
{
    obj.tileId = tio.tileId;
    obj.hFlip = tio.hFlip;
    obj.vFlip = tio.vFlip;
}

static std::vector<OverlappingObject> buildOverlappingObjects(const SI::Frame& siFrame)
{
    std::vector<OverlappingObject> ret;

    using f_iterator = std::vector<SI::FrameObject>::const_iterator;

    const auto& fobjs = siFrame.objects;

    for (f_iterator iIt = fobjs.begin(); iIt != fobjs.end(); ++iIt) {
        const SI::FrameObject& iObj = *iIt;

        std::vector<unsigned> underObjectIds;

        const urect iRect(iObj.location, iObj.sizePx());

        for (f_iterator jIt = iIt + 1; jIt != fobjs.end(); ++jIt) {
            const SI::FrameObject& jObj = *jIt;

            if (iRect.overlaps(jObj.location, jObj.sizePx())) {
                unsigned dj = std::distance(fobjs.begin(), jIt);
                underObjectIds.push_back(dj);
            }
        }

        if (not underObjectIds.empty()) {
            unsigned di = std::distance(fobjs.begin(), iIt);
            ret.emplace_back(OverlappingObject{ di, std::move(underObjectIds) });
        }
    }

    return ret;
}

// Also checks that there is no triple overlapping tiles
static std::array<bool, MAX_FRAME_OBJECTS>
buildOverlappingObjectsSieve(const SI::Frame& siFrame, const unsigned frameIndex, const std::vector<OverlappingObject>& overlappingObjects,
                             ErrorList& errorList)
{
    std::array<bool, MAX_FRAME_OBJECTS> sieve = {};

    assert(siFrame.objects.size() <= MAX_FRAME_OBJECTS);

    for (const auto& oo : overlappingObjects) {
        sieve.at(oo.overObjectId) = true;

        for (unsigned underObjId : oo.underObjectIds) {
            if (sieve.at(underObjId)) {
                errorList.addError(frameError(siFrame, frameIndex, u8"Cannot have three or more overlapping tiles"));
            }
            sieve.at(underObjId) = true;
        }
    }

    return sieve;
}

static MS::CollisionBox convertCollisionBox(const SI::CollisionBox& box, const upoint& siFrameOrigin)
{
    MS::CollisionBox ret;

    ret.exists = box.exists;

    if (box.exists) {
        ret.aabb = ms8rect::createFromOffset(box.aabb, siFrameOrigin);
    }

    return ret;
}

// processes everything except frameObject tiles
static void processFrameExcludingTiles(MS::Frame& msFrame, const SI::Frame& siFrame, const SI::FrameSetGrid& siGrid)
{
    msFrame.name = siFrame.name;

    const auto& siFrameOrigin = siFrame.origin(siGrid);

    msFrame.tileHitbox = convertCollisionBox(siFrame.tileHitbox, siFrameOrigin);
    msFrame.shield = convertCollisionBox(siFrame.shield, siFrameOrigin);
    msFrame.hitbox = convertCollisionBox(siFrame.hitbox, siFrameOrigin);
    msFrame.hurtbox = convertCollisionBox(siFrame.hurtbox, siFrameOrigin);

    for (const SI::FrameObject& siObj : siFrame.objects) {
        MS::FrameObject msObj;

        msObj.size = siObj.size;
        msObj.location = ms8point::createFromOffset(siObj.location, siFrameOrigin);

        msFrame.objects.emplace_back(msObj);
    }

    for (const SI::ActionPoint& siAp : siFrame.actionPoints) {
        MS::ActionPoint msAp;

        msAp.location = ms8point::createFromOffset(siAp.location, siFrameOrigin);
        msAp.type = siAp.type;

        msFrame.actionPoints.push_back(msAp);
    }

    msFrame.spriteOrder = siFrame.spriteOrder;
}

// Process frame object tiles not in sieve
static void processNotOverlappingTiles(MS::Frame& msFrame, TileExtractor& tileExtractor,
                                       const SI::Frame& siFrame, const urect& siFrameAabb, std::array<bool, MAX_FRAME_OBJECTS> sieve)
{
    assert(msFrame.objects.size() == siFrame.objects.size());

    for (auto [objectId, siObj] : const_enumerate(siFrame.objects)) {
        auto& msObj = msFrame.objects.at(objectId);

        if (sieve.at(objectId) == false) {
            const auto to = tileExtractor.getTilesetOutputFromImage(siFrameAabb, siObj);
            applyTilesetOutput(to, msObj);
        }
        else {
            msObj.tileId = UINT_MAX;
        }
    }
}

template <size_t OVER_SIZE>
static void _processOverlappingTiles(MS::Frame& msFrame, TileExtractor& tileExtractor,
                                     const SI::Frame& siFrame, const unsigned frameIndex, const urect& siFrameAabb,
                                     const OverlappingObject& oo,
                                     ErrorList& errorList)
{
    Snes::Tile<OVER_SIZE> overTile;

    const SI::FrameObject& siOverObj = siFrame.objects.at(oo.overObjectId);
    MS::FrameObject& msOverObj = msFrame.objects.at(oo.overObjectId);

    assert(msOverObj.tileId == UINT_MAX);

    overTile = tileExtractor.getTileFromImage<OVER_SIZE>(siFrameAabb, siOverObj);

    for (const unsigned underObjId : oo.underObjectIds) {
        const SI::FrameObject& siUnderObj = siFrame.objects.at(underObjId);
        MS::FrameObject& msUnderObj = msFrame.objects.at(underObjId);

        assert(msUnderObj.tileId == UINT_MAX);

        int xOffset = int(siOverObj.location.x) - int(siUnderObj.location.x);
        int yOffset = int(siOverObj.location.y) - int(siUnderObj.location.y);

        /*
         * This:
         *   - Gets the undertile pixels
         *   - Mark the undertile pixels that are overlapped with 0xFF
         *   - Search for duplicate tiles, creating a new tile if necessary
         */
        std::pair<Snes::TilesetInserterOutput, bool> tilesetOutput;
        if (siUnderObj.size == ObjectSize::SMALL) {
            auto underTile = tileExtractor.getTileFromImage<8>(siFrameAabb, siUnderObj);
            auto overlaps = markOverlappedPixels<OVER_SIZE, 8>(overTile, xOffset, yOffset);
            tilesetOutput = tileExtractor.smallTileset.processOverlappedTile(underTile, overlaps);
        }
        else {
            auto underTile = tileExtractor.getTileFromImage<16>(siFrameAabb, siUnderObj);
            auto overlaps = markOverlappedPixels<OVER_SIZE, 16>(overTile, xOffset, yOffset);
            tilesetOutput = tileExtractor.largeTileset.processOverlappedTile(underTile, overlaps);
        }
        applyTilesetOutput(tilesetOutput.first, msUnderObj);

        if (tilesetOutput.second == false) {
            errorList.addWarning(frameObjectError(siFrame, frameIndex, underObjId, u8"Matching undertile not found"));
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
        errorList.addWarning(frameObjectError(siFrame, frameIndex, oo.overObjectId, u8"Overtile is empty - skipping"));
    }
}

// To be called after processNotOverlappingTiles has been called on all frames in the frameSet
static void processOverlappingTiles(MS::Frame& msFrame, TileExtractor& tileExtractor,
                                    const SI::Frame& siFrame, const unsigned frameIndex, const urect& siFrameAabb,
                                    const std::vector<OverlappingObject>& overlappingObjects,
                                    ErrorList& errorList)
{
    for (auto& oo : overlappingObjects) {
        const SI::FrameObject& siOverObj = siFrame.objects.at(oo.overObjectId);

        if (siOverObj.size == ObjectSize::SMALL) {
            _processOverlappingTiles<8>(msFrame, tileExtractor, siFrame, frameIndex, siFrameAabb, oo, errorList);
        }
        else {
            _processOverlappingTiles<16>(msFrame, tileExtractor, siFrame, frameIndex, siFrameAabb, oo, errorList);
        }
    }
}

static void removeEmptyFrameObjects(std::vector<MS::FrameObject>& msFrameObjects)
{
    msFrameObjects.erase(
        std::remove_if(msFrameObjects.begin(), msFrameObjects.end(),
                       [](const MS::FrameObject& obj) { return obj.tileId == UINT_MAX; }),
        msFrameObjects.end());
}

static void processFrames(NamedList<MS::Frame>& msFrames, TileExtractor& tileExtractor,
                          const NamedList<SI::Frame>& siFrames, const SI::FrameSetGrid& siGrid, ErrorList& errorList)
{
    const auto oldErrorCount = errorList.errorCount();

    assert(msFrames.empty());

    std::vector<OverlappingObjectFrame> overlappingObjectFrames;

    for (auto [frameIndex, siFrame] : const_enumerate(siFrames)) {
        const auto frameOldErrorCount = errorList.errorCount();

        msFrames.insert_back();
        auto& msFrame = msFrames.back();

        const auto overlappingObjects = buildOverlappingObjects(siFrame);
        const auto sieve = buildOverlappingObjectsSieve(siFrame, frameIndex, overlappingObjects, errorList);

        if (errorList.errorCount() != frameOldErrorCount) {
            continue;
        }

        const auto siFrameAabb = siFrame.frameLocation(siGrid);

        processFrameExcludingTiles(msFrame, siFrame, siGrid);
        processNotOverlappingTiles(msFrame, tileExtractor, siFrame, siFrameAabb, sieve);

        if (not overlappingObjects.empty()) {
            overlappingObjectFrames.emplace_back(OverlappingObjectFrame{ unsigned(frameIndex), std::move(overlappingObjects) });
        }
    }

    if (errorList.errorCount() != oldErrorCount) {
        // do not process overlapping tiles if there is an error
        return;
    }

    // process overlapping tiles
    for (auto& oof : overlappingObjectFrames) {
        const SI::Frame& siFrame = siFrames.at(oof.frameIndex);
        MS::Frame& msFrame = msFrames.at(oof.frameIndex);

        const auto siFrameAabb = siFrame.frameLocation(siGrid);

        processOverlappingTiles(msFrame, tileExtractor, siFrame, oof.frameIndex, siFrameAabb, oof.overlappingObjects, errorList);

        removeEmptyFrameObjects(msFrame.objects);
    }
}

}
