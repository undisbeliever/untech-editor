/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framesetcompiler.h"
#include "animationcompiler.h"
#include "compiler.h"
#include "palettecompiler.h"
#include "tilesetinserter.h"
#include "tilesetlayout.h"
#include "models/common/errorlist.h"
#include "models/metasprite/utsi2utms/utsi2utms.h"
#include "models/project/project.h"

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

namespace MS = UnTech::MetaSprite::MetaSprite;

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

    ms8rect outerAabb;
    for (const MS::EntityHitbox& eh : entityHitboxes) {
        outerAabb.extend(eh.aabb);
    }
    assert(outerAabb.width > 0 && outerAabb.height > 0);

    // count starts at -1
    unsigned count = entityHitboxes.size() - 1;
    unsigned dataSize = 5 + 5 * entityHitboxes.size();

    // a Hitbox with a single aabb is a special case.
    if (count == 0) {
        dataSize = 5 + 1;
    }
    romData.reserve(dataSize);

    romData.push_back(count); // count

    romData.push_back(outerAabb.x.romData()); // Outer::xOffset
    romData.push_back(outerAabb.width - 1);   // Outer::width
    romData.push_back(outerAabb.y.romData()); // Outer::yOffset
    romData.push_back(outerAabb.height - 1);  // Outer::height

    if (count > 0) {
        for (const MS::EntityHitbox& eh : entityHitboxes) {
            const ms8rect& innerAabb = eh.aabb;

            assert(innerAabb.width > 0 && innerAabb.height > 0);

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

static std::vector<uint8_t> processActionPoints(const std::vector<MS::ActionPoint>& actionPoints,
                                                const ActionPointMapping& actionPointMapping)
{
    assert(actionPoints.size() <= MAX_ACTION_POINTS);

    std::vector<uint8_t> romData;

    if (actionPoints.size() == 0) {
        return romData;
    }

    romData.reserve(3 * actionPoints.size() + 1);

    for (const MS::ActionPoint& ap : actionPoints) {
        const ms8point loc = ap.location;

        romData.push_back(actionPointMapping.at(ap.type)); // ActionPoint::type
        romData.push_back(loc.x.romData());                // ActionPoint::positionPair.xPos
        romData.push_back(loc.y.romData());                // ActionPoint::positionPair.yPos
    }

    romData.push_back(0); // null terminator

    return romData;
}

static TileHitboxData processTileHitbox(const MS::Frame& frame)
{
    if (frame.solid) {
        const ms8rect& hb = frame.tileHitbox;

        const auto left = -hb.left();
        const auto right = hb.right();
        const auto yOffset = -hb.top();
        const auto height = hb.height;

        auto assert_bounds = [](int v) {
            assert(v >= 1 && v <= 127);
        };
        assert_bounds(left);
        assert_bounds(right);
        assert_bounds(yOffset);
        assert_bounds(height);
        assert(int(height) - yOffset > 0);

        return {
            .left = uint8_t(left),
            .right = uint8_t(right),
            .yOffset = uint8_t((yOffset ^ 0xff) + 1),
            .height = height,
        };
    }
    else {
        return { 0xff, 0xff, 0xff, 0xff };
    }
}

static FrameData processFrame(const MS::Frame& frame,
                              const std::optional<unsigned> tilesetIndex, const FrameTilesetData& frameTileset,
                              const ActionPointMapping& actionPointMapping)
{
    return {
        .frameObjects = processFrameObjects(frame, frameTileset),
        .entityHitboxes = processEntityHitboxes(frame.entityHitboxes),
        .actionPoints = processActionPoints(frame.actionPoints, actionPointMapping),
        .tileset = tilesetIndex,
        .tileHitbox = processTileHitbox(frame),
    };
}

static FrameData processFrame(const FrameListEntry& fle,
                              const std::optional<unsigned> tilesetIndex, const FrameTilesetData& frameTileset,
                              const ActionPointMapping& actionPointMapping)
{
    if (fle.hFlip == false && fle.vFlip == false) {
        return processFrame(*fle.frame, tilesetIndex, frameTileset, actionPointMapping);
    }
    else {
        auto flippedFrame = fle.frame->flip(fle.hFlip, fle.vFlip);
        return processFrame(flippedFrame, tilesetIndex, frameTileset, actionPointMapping);
    }
}

static std::vector<FrameData> processFrameList(const FrameSetExportList& exportList, const TilesetData& tilesetData,
                                               const ActionPointMapping& actionPointMapping)
{
    const auto& frameList = exportList.frames;

    std::vector<FrameData> frames;
    frames.reserve(frameList.size());

    for (unsigned frameId = 0; frameId < frameList.size(); frameId++) {
        const auto& fle = frameList.at(frameId);
        assert(fle.frame);

        const auto& tilesetIndex = tilesetData.tilesetIndexForFrameId(frameId);
        const auto& frameTileset = tilesetData.getTileset(tilesetIndex);
        frames.push_back(processFrame(fle, tilesetIndex, frameTileset, actionPointMapping));
    }

    return frames;
}

static std::shared_ptr<const FrameSetData>
compileFrameSet(const MetaSprite::FrameSet& frameSet,
                const Project::ProjectFile& project, const ActionPointMapping& actionPointMapping,
                ErrorList& errorList)
{
    const size_t oldErrorCount = errorList.errorCount();

    const auto* exportOrder = project.frameSetExportOrders.find(frameSet.exportOrder);
    if (exportOrder == nullptr) {
        errorList.addErrorString("Missing MetaSprite Export Order Document");
        return nullptr;
    }

    const bool valid = frameSet.validate(actionPointMapping, errorList)
                       && exportOrder->testFrameSet(frameSet, errorList);

    if (!valid) {
        return nullptr;
    }

    const FrameSetExportList exportList = buildExportList(frameSet, *exportOrder);
    exportList.validate(errorList);

    auto out = std::make_shared<FrameSetData>();
    const auto tilesetLayout = layoutTiles(frameSet, exportList.frames, errorList);

    out->tileset = processTileset(frameSet, tilesetLayout);
    out->frames = processFrameList(exportList, out->tileset, actionPointMapping);
    out->animations = processAnimations(exportList);
    out->palettes = processPalettes(frameSet.palettes);

    if (errorList.errorCount() != oldErrorCount) {
        out = nullptr;
    }

    return out;
}

std::shared_ptr<const FrameSetData>
compileFrameSet(const FrameSetFile& fs,
                const Project::ProjectFile& project, const ActionPointMapping& actionPointMapping,
                ErrorList& errorList)
{
    if (fs.msFrameSet) {
        return compileFrameSet(*fs.msFrameSet, project, actionPointMapping, errorList);
    }
    else if (fs.siFrameSet) {
        if (const auto msfs = utsi2utms(*fs.siFrameSet, errorList)) {
            return compileFrameSet(*msfs, project, actionPointMapping, errorList);
        }
    }
    else {
        errorList.addErrorString("Missing FrameSet");
    }

    return nullptr;
}

}
}
}
