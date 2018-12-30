/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framecompiler.h"
#include "compiler.h"
#include "tilesetinserter.h"

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

namespace MS = UnTech::MetaSprite::MetaSprite;

constexpr uint32_t NULL_OFFSET = ~0;

static std::vector<uint8_t> processFrameObjects(const MS::Frame& frame,
                                                const FrameTilesetData& tileMap)
{
    const static uint16_t V_FLIP = 0x8000;
    const static uint16_t H_FLIP = 0x4000;
    const static unsigned ORDER_SHIFT = 12;

    const size_t nObjects = frame.objects.size();

    assert(nObjects <= MAX_FRAME_OBJECTS);

    std::vector<uint8_t> romData;

    if (nObjects == 0) {
        return romData;
    }

    romData.reserve(1 + 4 * nObjects);
    romData.push_back(nObjects - 1); // count

    for (const MS::FrameObject& obj : frame.objects) {
        const ms8point loc = obj.location;

        uint16_t charAttr = obj.size == ObjectSize::SMALL ? tileMap.smallTilesCharAttr.at(obj.tileId)
                                                          : tileMap.largeTilesCharAttr.at(obj.tileId);
        assert(charAttr != FrameTilesetData::NULL_CHAR_ATTR);

        charAttr |= frame.spriteOrder << ORDER_SHIFT;

        if (obj.hFlip) {
            charAttr ^= H_FLIP;
        }

        if (obj.vFlip) {
            charAttr ^= V_FLIP;
        }

        romData.push_back(loc.x.romData());        // Objects::xOffset
        romData.push_back(loc.y.romData());        // Objects::yOffset
        romData.push_back(charAttr & 0xFF);        // Objects::char
        romData.push_back((charAttr >> 8) & 0xFF); // Objects::attr
    }

    return romData;
}

static std::vector<uint8_t> processEntityHitboxes(const std::vector<MS::EntityHitbox>& entityHitboxes)
{
    assert(entityHitboxes.size() <= MAX_ENTITY_HITBOXES);

    std::vector<uint8_t> romData;

    if (entityHitboxes.size() == 0) {
        return romData;
    }

    // count starts at -1
    unsigned count = entityHitboxes.size() - 1;
    unsigned dataSize = 5 + 5 * entityHitboxes.size();

    // a Hitbox with a single aabb is a special case.
    if (count == 0) {
        dataSize = 5 + 1;
    }
    romData.reserve(dataSize);

    ms8rect outerAabb;
    {
        // calculate outer AABB.
        for (const MS::EntityHitbox& eh : entityHitboxes) {
            outerAabb.extend(eh.aabb);
        }

        if (outerAabb.width == 0 || outerAabb.height == 0) {
            throw std::runtime_error("Entity Hitbox aabb has no size");
        }
    }

    romData.push_back(count); // count

    romData.push_back(outerAabb.x.romData()); // Outer::xOffset
    romData.push_back(outerAabb.width - 1);   // Outer::width
    romData.push_back(outerAabb.y.romData()); // Outer::yOffset
    romData.push_back(outerAabb.height - 1);  // Outer::height

    if (count > 0) {
        for (const MS::EntityHitbox& eh : entityHitboxes) {
            const ms8rect& innerAabb = eh.aabb;

            if (innerAabb.width == 0 || innerAabb.height == 0) {
                throw std::runtime_error("Entity Hitbox aabb has no size");
            }

            romData.push_back(eh.hitboxType.romValue()); // Inner:type
            romData.push_back(innerAabb.x.romData());    // Inner::xOffset
            romData.push_back(innerAabb.width - 1);      // Inner::width
            romData.push_back(innerAabb.y.romData());    // Inner::yOffset
            romData.push_back(innerAabb.height - 1);     // Inner::height
        }
    }
    else {
        const MS::EntityHitbox& eh = entityHitboxes.at(0);
        romData.push_back(eh.hitboxType.romValue()); // SingleHitbox::type
    }

    return romData;
}

static std::vector<uint8_t> processTileHitbox(const MS::Frame& frame)
{
    const ms8rect& aabb = frame.tileHitbox;

    if (aabb.width == 0 || aabb.height == 0) {
        throw std::runtime_error("Tileset Hitbox aabb has no size");
    }

    std::vector<uint8_t> romData;
    if (frame.solid == false) {
        return romData;
    }

    romData.reserve(4);

    romData.push_back(aabb.x.romData()); // xOffset
    romData.push_back(aabb.y.romData()); // yOffset
    romData.push_back(aabb.width);       // width
    romData.push_back(aabb.height);      // height

    return romData;
}

static std::vector<uint8_t> processActionPoints(const std::vector<MS::ActionPoint>& actionPoints)
{
    assert(actionPoints.size() <= MAX_ACTION_POINTS);

    std::vector<uint8_t> romData;

    if (actionPoints.size() == 0) {
        return romData;
    }

    romData.reserve(3 * actionPoints.size() + 1);

    for (const MS::ActionPoint& ap : actionPoints) {
        assert(ap.parameter != 0);

        const ms8point loc = ap.location;

        romData.push_back(ap.parameter);    // Point::parameter
        romData.push_back(loc.x.romData()); // Point::xOffset
        romData.push_back(loc.y.romData()); // Point::yOffset
    }

    romData.push_back(0); // null terminator

    return romData;
}

static FrameData processFrame(const MS::Frame& frame, const FrameTilesetData& frameTileset)
{
    RomOffsetPtr tilesetAddr;
    if (frameTileset.dynamicTileset) {
        tilesetAddr = frameTileset.romPtr;
    }

    return {
        .frameObjects = processFrameObjects(frame, frameTileset),
        .entityHitboxes = processEntityHitboxes(frame.entityHitboxes),
        .tileHitbox = processTileHitbox(frame),
        .actionPoints = processActionPoints(frame.actionPoints),
        .tileset = tilesetAddr,
        .isNull = false,
    };
}

static FrameData processFrame(const FrameListEntry& fle, const FrameTilesetData& frameTileset)
{
    if (fle.hFlip == false && fle.vFlip == false) {
        return processFrame(*fle.frame, frameTileset);
    }
    else {
        auto flippedFrame = fle.frame->flip(fle.hFlip, fle.vFlip);
        return processFrame(flippedFrame, frameTileset);
    }
}

std::vector<FrameData> processFrameList(const FrameSetExportList& exportList, const TilesetData& tilesetData,
                                        ErrorList& errorList)
{
    const auto& frameList = exportList.frames;

    std::vector<FrameData> frames;
    frames.reserve(frameList.size());

    for (unsigned frameId = 0; frameId < frameList.size(); frameId++) {
        const auto& fle = frameList.at(frameId);
        const auto& frameTileset = tilesetData.tilesetForFrameId(frameId);

        if (fle.frame != nullptr) {
            try {
                frames.push_back(processFrame(fle, frameTileset));
            }
            catch (const std::exception& ex) {
                errorList.addError(exportList.frameSet, *fle.frame, ex.what());

                frames.emplace_back();
                frames.back().isNull = true;
            }
        }
    }

    return frames;
}

static uint32_t saveCompiledFrame(const FrameData& frameData, CompiledRomData& out)
{
    if (frameData.isNull) {
        return NULL_OFFSET;
    }

    RomIncItem data;

    data.addAddr(out.frameObjectData.addData(frameData.frameObjects));
    data.addAddr(out.entityHitboxData.addData(frameData.entityHitboxes));
    data.addAddr(out.tileHitboxData.addData(frameData.tileHitbox));
    data.addAddr(out.actionPointData.addData(frameData.actionPoints));
    data.addAddr(frameData.tileset);

    return out.frameData.addData(data).offset;
}

RomOffsetPtr saveCompiledFrames(const std::vector<FrameData>& framesData, CompiledRomData& out)
{
    std::vector<uint32_t> frameOffsets(framesData.size());

    std::transform(framesData.begin(), framesData.end(),
                   frameOffsets.begin(),
                   [&](auto& fd) { return saveCompiledFrame(fd, out); });

    return out.frameList.getOrInsertTable(frameOffsets);
}

}
}
}
