/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framecompiler.h"

using namespace UnTech::MetaSprite;
using namespace UnTech::MetaSprite::Compiler;

namespace MS = UnTech::MetaSprite::MetaSprite;

FrameCompiler::FrameCompiler(ErrorList& errorList)
    : _errorList(errorList)
    , _frameData("FD", "MS_FrameData")
    , _frameList("FL", "MS_FrameList", "FD")
    , _frameObjectData("FO", "MS_FrameObjectsData", true)
    , _tileHitboxData("TC", "MS_TileHitboxData", true)
    , _entityHitboxData("EH", "MS_EntityHitboxData", true)
    , _actionPointData("AP", "MS_ActionPointsData", true)
{
}

void FrameCompiler::writeToIncFile(std::ostream& out) const
{
    _frameObjectData.writeToIncFile(out);
    _tileHitboxData.writeToIncFile(out);
    _actionPointData.writeToIncFile(out);
    _entityHitboxData.writeToIncFile(out);

    _frameData.writeToIncFile(out);
    _frameList.writeToIncFile(out);
}

inline RomOffsetPtr
FrameCompiler::processFrameObjects(const MS::Frame& frame,
                                   const FrameTileset& tileset)
{
    const static uint16_t V_FLIP = 0x8000;
    const static uint16_t H_FLIP = 0x4000;
    const static unsigned ORDER_SHIFT = 12;

    size_t nObjects = frame.objects.size();

    if (nObjects == 0) {
        return RomOffsetPtr();
    }

    assert(nObjects <= MAX_FRAME_OBJECTS);

    std::vector<uint8_t> romData;
    romData.reserve(1 + 4 * nObjects);

    romData.push_back(nObjects - 1); // count

    for (const MS::FrameObject& obj : frame.objects) {
        const ms8point loc = obj.location;

        uint16_t charAttr;

        if (obj.size == ObjectSize::SMALL) {
            charAttr = tileset.smallTilesetMap.at(obj.tileId);
        }
        else {
            charAttr = tileset.largeTilesetMap.at(obj.tileId);
        }

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

    return _frameObjectData.addData(romData);
}

inline RomOffsetPtr
FrameCompiler::processEntityHitboxes(const MS::EntityHitbox::list_t& entityHitboxes)
{
    if (entityHitboxes.size() == 0) {
        return RomOffsetPtr();
    }

    assert(entityHitboxes.size() <= MAX_ENTITY_HITBOXES);

    // count starts at -1
    unsigned count = entityHitboxes.size() - 1;
    unsigned dataSize = 7 + 7 * entityHitboxes.size();

    // a Hitbox with a single aabb is a special case.
    if (count == 0) {
        dataSize = 7 + 1;
    }

    std::vector<uint8_t> romData;
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
    romData.push_back(outerAabb.y.romData()); // Outer::yOffset
    romData.push_back(outerAabb.width);       // Outer::width
    romData.push_back(0);                     // Outer::width high byte
    romData.push_back(outerAabb.height);      // Outer::height
    romData.push_back(0);                     // Outer::height high byte

    if (count > 0) {
        for (const MS::EntityHitbox& eh : entityHitboxes) {
            const ms8rect& innerAabb = eh.aabb;

            if (innerAabb.width == 0 || innerAabb.height == 0) {
                throw std::runtime_error("Entity Hitbox aabb has no size");
            }

            romData.push_back(eh.hitboxType.romValue()); // Inner:type
            romData.push_back(innerAabb.x.romData());    // Inner::xOffset
            romData.push_back(innerAabb.y.romData());    // Inner::yOffset
            romData.push_back(innerAabb.width);          // Inner::width
            romData.push_back(0);                        // Inner::width high byte
            romData.push_back(innerAabb.height);         // Inner::height
            romData.push_back(0);                        // Inner::height high byte
        }
    }
    else {
        const MS::EntityHitbox& eh = entityHitboxes.at(0);
        romData.push_back(eh.hitboxType.romValue()); // SingleHitbox::type
    }

    return _entityHitboxData.addData(romData);
}

inline RomOffsetPtr
FrameCompiler::processTileHitbox(const MS::Frame& frame)
{
    if (frame.solid == false) {
        return RomOffsetPtr();
    }

    const ms8rect& aabb = frame.tileHitbox;

    if (aabb.width == 0 || aabb.height == 0) {
        throw std::runtime_error("Tileset Hitbox aabb has no size");
    }

    std::vector<uint8_t> romData(4);

    romData.push_back(aabb.x.romData()); // xOffset
    romData.push_back(aabb.y.romData()); // yOffset
    romData.push_back(aabb.width);       // width
    romData.push_back(aabb.height);      // height

    return _tileHitboxData.addData(romData);
}

inline RomOffsetPtr
FrameCompiler::processActionPoints(const MS::ActionPoint::list_t& actionPoints)
{
    if (actionPoints.size() == 0) {
        return RomOffsetPtr();
    }

    assert(actionPoints.size() <= MAX_ACTION_POINTS);

    std::vector<uint8_t> romData;
    romData.reserve(3 * actionPoints.size() + 1);

    for (const MS::ActionPoint& ap : actionPoints) {
        assert(ap.parameter != 0);

        const ms8point loc = ap.location;

        romData.push_back(ap.parameter);    // Point::parameter
        romData.push_back(loc.x.romData()); // Point::xOffset
        romData.push_back(loc.y.romData()); // Point::yOffset
    }

    romData.push_back(0); // null terminator

    return _actionPointData.addData(romData);
}

inline uint32_t
FrameCompiler::processFrame(const MS::Frame& frame, const FrameTileset& tileset)
{
    RomOffsetPtr frameObjects = processFrameObjects(frame, tileset);
    RomOffsetPtr enityHitbox = processEntityHitboxes(frame.entityHitboxes);
    RomOffsetPtr tileHitbox = processTileHitbox(frame);
    RomOffsetPtr actionPoints = processActionPoints(frame.actionPoints);

    RomIncItem data;
    data.addAddr(frameObjects);
    data.addAddr(enityHitbox);
    data.addAddr(tileHitbox);
    data.addAddr(actionPoints);
    data.addAddr(tileset.tilesetOffset);

    return _frameData.addData(data).offset;
}

RomOffsetPtr
FrameCompiler::process(const FrameSetExportList& exportList,
                       const FrameTilesetList& tilesets)
{
    const auto& frameList = exportList.frames();

    assert(frameList.size() <= MAX_EXPORT_NAMES);

    const uint32_t NULL_OFFSET = ~0;

    std::vector<uint32_t> frameOffsets;
    frameOffsets.reserve(frameList.size());

    for (const auto& fle : frameList) {
        if (fle.frame != nullptr) {
            try {
                uint32_t fo = NULL_OFFSET;
                const FrameTileset& frameTileset = tilesets.frameMap.at(fle.frame);

                if (fle.hFlip == false && fle.vFlip == false) {
                    fo = processFrame(*fle.frame, frameTileset);
                }
                else {
                    auto flippedFrame = fle.frame->flip(fle.hFlip, fle.vFlip);
                    fo = processFrame(flippedFrame, frameTileset);
                }

                frameOffsets.push_back(fo);
            }
            catch (const std::exception& ex) {
                _errorList.addError(exportList.frameSet(), *fle.frame, ex.what());

                frameOffsets.push_back(NULL_OFFSET);
            }
        }
        else {
            frameOffsets.push_back(NULL_OFFSET);
        }
    }

    return _frameList.getOrInsertTable(frameOffsets);
}
