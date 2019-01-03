/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "spriteimporter.h"
#include "errorlisthelpers.h"
#include "models/common/errorlist.h"
#include "models/common/file.h"
#include "models/common/imagecache.h"
#include <algorithm>

using namespace UnTech;
using namespace UnTech::MetaSprite::SpriteImporter;

/*
 * FRAME SET GRID
 * ==============
 */

bool FrameSetGrid::isValid(const FrameSet& frameSet) const
{
    usize minSize = frameSet.minimumFrameGridSize();

    return frameSize.width >= minSize.width
           && frameSize.height >= minSize.height
           && frameSize.width <= MAX_FRAME_SIZE
           && frameSize.height <= MAX_FRAME_SIZE
           && origin.x <= MAX_ORIGIN
           && origin.y <= MAX_ORIGIN
           && frameSize.contains(origin);
}

urect FrameSetGrid::cell(unsigned x, unsigned y) const
{
    return urect(
        x * (frameSize.width + padding.x) + offset.x,
        y * (frameSize.height + padding.y) + offset.y,
        frameSize.width,
        frameSize.height);
}

usize FrameSetGrid::originRange() const
{
    return usize(
        std::min(MAX_ORIGIN, frameSize.width),
        std::min(MAX_ORIGIN, frameSize.height));
}

bool FrameSetGrid::operator==(const FrameSetGrid& o) const
{
    return frameSize == o.frameSize
           && offset == o.offset
           && padding == o.padding
           && origin == o.origin;
}

/*
 * FRAME LOCATION
 * ==============
 */

void FrameLocation::update(const FrameSetGrid& grid, const Frame& frame)
{
    if (useGridLocation) {
        aabb = grid.cell(gridLocation.x, gridLocation.y);
    }

    usize minSize = frame.minimumViableSize();

    aabb.width = std::max(aabb.width, minSize.width);
    aabb.height = std::max(aabb.height, minSize.height);

    if (useGridOrigin) {
        origin = grid.origin;
    }
    origin.x = std::min(origin.x, aabb.width);
    origin.y = std::min(origin.y, aabb.height);
}

bool FrameLocation::isValid(const FrameSet& frameSet, const Frame& frame)
{
    update(frameSet.grid, frame);

    usize minSize = frame.minimumViableSize();

    return aabb.width >= minSize.width
           && aabb.height >= minSize.height
           && aabb.width <= MAX_FRAME_SIZE
           && aabb.height <= MAX_FRAME_SIZE
           && origin.x <= MAX_ORIGIN
           && origin.y <= MAX_ORIGIN
           && aabb.size().contains(origin);
}

usize FrameLocation::originRange() const
{
    return usize(
        std::min(MAX_ORIGIN, aabb.width),
        std::min(MAX_ORIGIN, aabb.height));
}

bool FrameLocation::operator==(const FrameLocation& o) const
{
    return this->aabb == o.aabb
           && this->origin == o.origin
           && this->gridLocation == o.gridLocation
           && this->useGridLocation == o.useGridLocation
           && this->useGridOrigin == o.useGridOrigin;
}

/*
 * FRAME
 * =====
 */

bool Frame::validate(ErrorList& errorList, const FrameSet& fs)
{
    bool valid = true;

    auto addError = [&](const std::string& msg) {
        errorList.addError(frameError(fs, *this, msg));
        valid = false;
    };

    if (objects.size() > MAX_FRAME_OBJECTS) {
        addError("Too many frame objects");
    }
    if (actionPoints.size() > MAX_ACTION_POINTS) {
        addError("Too many action points");
    }
    if (entityHitboxes.size() > MAX_ENTITY_HITBOXES) {
        addError("Too many entity hitboxes");
    }

    if (location.isValid(fs, *this) == false) {
        addError("Invalid Frame Size");
    }

    auto image = ImageCache::loadPngImage(fs.imageFilename);
    if (image->size().contains(location.aabb) == false) {
        addError("Frame not inside image");
    }

    for (const FrameObject& obj : objects) {
        if (obj.isValid(location) == false) {
            addError("Frame Object not inside frame");
        }
    }

    for (const ActionPoint& ap : actionPoints) {
        if (ap.isValid(location) == false) {
            addError("Action Point not inside frame");
        }
    }

    for (const EntityHitbox& eh : entityHitboxes) {
        if (eh.isValid(location) == false) {
            addError("Entity Hitbox aabb invalid");
        }
    }

    return valid;
}

usize Frame::minimumViableSize() const
{
    usize limit = usize(MIN_FRAME_SIZE, MIN_FRAME_SIZE);

    limit = limit.expand(location.origin);

    if (solid) {
        limit = limit.expand(tileHitbox);
    }

    for (const auto& obj : objects) {
        limit = limit.expand(obj.bottomRight());
    }

    for (const auto& ap : actionPoints) {
        limit = limit.expand(ap.location);
    }

    for (const auto& eh : entityHitboxes) {
        limit = limit.expand(eh.aabb);
    }

    return limit;
}

bool Frame::operator==(const Frame& o) const
{
    return location == o.location
           && objects == o.objects
           && actionPoints == o.actionPoints
           && entityHitboxes == o.entityHitboxes
           && tileHitbox == o.tileHitbox
           && spriteOrder == o.spriteOrder
           && solid == o.solid;
}

/*
 * FRAME SET
 * =========
 */

bool FrameSet::validate(ErrorList& errorList)
{
    bool valid = true;
    auto addError = [&](std::string&& msg) {
        errorList.addError(msg);
        valid = false;
    };

    // Validate FrameSet

    if (name.isValid() == false) {
        addError("Missing name");
    }
    if (exportOrder.isValid() == false) {
        addError("Missing exportOrder");
    }
    if (frames.size() == 0) {
        addError("No Frames");
    }
    if (imageFilename.empty()) {
        addError("No Image");
    }
    if (transparentColorValid() == false) {
        addError("Transparent color is invalid");
    }
    if (grid.isValid(*this) == false) {
        addError("Invalid Frame Set Grid");
    }

    if (valid == false) {
        return false;
    }

    if (palette.usesUserSuppliedPalette()) {
        auto image = ImageCache::loadPngImage(imageFilename);

        auto imgSize = image->size();
        auto palSize = palette.paletteSize();

        if (palette.nPalettes > MAX_PALETTES) {
            addError("Too many palettes");
        }

        if (palSize.width > imgSize.width
            || palSize.height > imgSize.height) {

            addError("Palette outside image");
        }
    }

    if (animations.size() > MAX_ANIMATION_FRAMES) {
        addError("Too many animations in frameSet");
    }

    for (auto&& it : frames) {
        valid &= it.second.validate(errorList, *this);
    }

    for (auto&& it : animations) {
        valid &= it.second.validate(*this, errorList);
    }

    return valid;
}

usize FrameSet::minimumFrameGridSize() const
{
    usize limit = usize(MIN_FRAME_SIZE, MIN_FRAME_SIZE);

    for (const auto& it : frames) {
        const Frame& frame = it.second;

        if (frame.location.useGridLocation) {
            limit = limit.expand(frame.minimumViableSize());
        }
    }

    return limit;
}

void FrameSet::updateFrameLocations()
{
    for (auto&& it : frames) {
        it.second.location.update(grid, it.second);
    }
}

bool FrameSet::operator==(const FrameSet& o) const
{
    auto testMap = [](const auto& aMap, const auto& bMap) -> bool {
        for (const auto& aIt : aMap) {
            const auto* bValue = bMap.getPtr(aIt.first);

            if (bValue == nullptr || aIt.second != *bValue) {
                return false;
            }
        }

        return true;
    };

    auto testTransparentColor = [&]() -> bool {
        if (transparentColorValid() != o.transparentColorValid()) {
            return false;
        }
        else if (transparentColorValid()) {
            return transparentColor == transparentColor;
        }
        else {
            return true;
        }
    };

    // Don't test image, imageFilename test is fine for now

    return name == o.name
           && tilesetType == o.tilesetType
           && exportOrder == o.exportOrder
           && imageFilename == o.imageFilename
           && palette == o.palette
           && grid == o.grid
           && testTransparentColor()
           && testMap(frames, o.frames)
           && testMap(animations, o.animations);
}
