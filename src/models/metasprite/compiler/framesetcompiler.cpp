/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framesetcompiler.h"
#include "animationcompiler.h"
#include "compiler.h"
#include "palettecompiler.h"
#include "tilesetinserter.h"
#include "tilesetlayout.h"
#include "models/common/errorlist.h"
#include "models/common/iterators.h"
#include "models/metasprite/utsi2utms/utsi2utms.h"
#include "models/project/project.h"

namespace UnTech::MetaSprite::Compiler {

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

static std::array<uint8_t, 12> processCollisionBoxes(const MS::Frame& frame)
{
    std::array<uint8_t, 12> romData;

    auto it = romData.begin();

    auto writeBox = [&](const MS::CollisionBox& box) {
        if (box.exists) {
            assert(box.aabb.width > 0 && box.aabb.width <= MAX_COLLISION_BOX_SIZE);
            assert(box.aabb.height > 0 && box.aabb.height <= MAX_COLLISION_BOX_SIZE);

            *it++ = box.aabb.x.romData(); // xOffset
            *it++ = box.aabb.width - 1;   // width
            *it++ = box.aabb.y.romData(); // yOffset
            *it++ = box.aabb.height - 1;  // height
        }
        else {
            *it++ = 0xff; // xOffset
            *it++ = 0xff; // width
            *it++ = 0xff; // yOffset
            *it++ = 0xff; // height
        }
    };

    writeBox(frame.shield);
    writeBox(frame.hurtbox);
    writeBox(frame.hitbox);

    assert(it == romData.end());

    return romData;
}

static TileHitboxData processTileHitbox(const MS::Frame& frame)
{
    if (frame.tileHitbox.exists) {
        const ms8rect& hb = frame.tileHitbox.aabb;

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
        .actionPoints = processActionPoints(frame.actionPoints, actionPointMapping),
        .collisionBoxes = processCollisionBoxes(frame),
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

    for (auto [frameId, fle] : const_enumerate(frameList)) {
        assert(fle.frame);

        const auto& tilesetIndex = tilesetData.tilesetIndexForFrameId(frameId);
        const auto& frameTileset = tilesetData.getTileset(tilesetIndex);
        frames.push_back(processFrame(fle, tilesetIndex, frameTileset, actionPointMapping));
    }

    return frames;
}

static void checkExportListSize(const FrameSetExportList& input, ErrorList& err)
{
    if (input.animations.size() > MAX_EXPORT_NAMES) {
        err.addErrorString("Too many animations (", input.animations.size(), ", max ", MAX_EXPORT_NAMES, ")");
    }
    if (input.frames.size() > MAX_EXPORT_NAMES) {
        err.addErrorString("Too many frames (", input.frames.size(), ", max ", MAX_EXPORT_NAMES, ")");
    }
}

static std::shared_ptr<FrameSetData>
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

    const bool valid = validate(frameSet, actionPointMapping, errorList)
                       && exportOrder->testFrameSet(frameSet, errorList);

    if (!valid) {
        return nullptr;
    }

    const FrameSetExportList exportList = buildExportList(frameSet, *exportOrder);
    checkExportListSize(exportList, errorList);

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
        if (auto msfs = utsi2utms(*fs.siFrameSet, errorList)) {
            auto fsData = compileFrameSet(*msfs, project, actionPointMapping, errorList);
            if (fsData) {
                fsData->msFrameSet = std::move(msfs);
            }
            return fsData;
        }
    }
    else {
        errorList.addErrorString("Missing FrameSet");
    }

    return nullptr;
}

}
