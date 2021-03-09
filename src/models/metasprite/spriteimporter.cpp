/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "spriteimporter.h"
#include "errorlisthelpers.h"
#include "models/common/errorlist.h"
#include "models/common/file.h"
#include "models/common/imagecache.h"
#include "models/common/iterators.h"
#include "models/common/validateunique.h"
#include <algorithm>

using namespace UnTech;
using namespace UnTech::MetaSprite::SpriteImporter;

const EnumMap<UserSuppliedPalette::Position> UserSuppliedPalette::positionEnumMap{ {
    { "TOP_LEFT", UserSuppliedPalette::Position::TOP_LEFT },
    { "TOP_RIGHT", UserSuppliedPalette::Position::TOP_RIGHT },
    { "BOTTOM_LEFT", UserSuppliedPalette::Position::BOTTOM_LEFT },
    { "BOTTOM_RIGHT", UserSuppliedPalette::Position::BOTTOM_RIGHT },
} };

/*
 * FRAME SET GRID
 * ==============
 */

inline bool FrameSetGrid::validate(ErrorList& errorList) const
{
    bool valid = true;
    auto addError = [&](const auto&... msg) {
        errorList.addErrorString(msg...);
        valid = false;
    };

    if (frameSize.width == 0 || frameSize.height == 0) {
        addError("grid.frameSize has no size");
    }
    if (frameSize.width > MAX_FRAME_SIZE || frameSize.height > MAX_FRAME_SIZE) {
        addError("grid.frameSize is too large (max: ", MAX_FRAME_SIZE, " x ", MAX_FRAME_SIZE, ")");
    }
    if (origin.x > MAX_ORIGIN || origin.y > MAX_ORIGIN) {
        addError("grid.origin is too large (max: ", MAX_ORIGIN, ", ", MAX_ORIGIN, ")");
    }
    if (frameSize.contains(origin) == false) {
        addError("grid.origin is not inside grid.frameSize");
    }

    return valid;
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

    if (useGridLocation == false) {
        // update gridLocation to match nearest grid cell
        if (grid.frameSize.width > 0 && grid.frameSize.height > 0) {
            unsigned x = aabb.x > grid.offset.x ? aabb.x - grid.offset.x : 0;
            unsigned y = aabb.y > grid.offset.y ? aabb.y - grid.offset.y : 0;

            gridLocation.x = x / grid.frameSize.width;
            gridLocation.y = y / grid.frameSize.height;
        }
    }

    if (useGridOrigin) {
        origin = grid.origin;
    }
    origin.x = std::min(origin.x, aabb.width);
    origin.y = std::min(origin.y, aabb.height);
}

inline bool FrameLocation::validate(ErrorList& errorList, const Frame& frame) const
{
    bool valid = true;
    auto addError = [&](const auto&... msg) {
        errorList.addError(frameError(frame, msg...));
        valid = false;
    };

    if (aabb.width == 0 || aabb.height == 0) {
        addError("FrameLocation aabb has no size");
    }
    if (aabb.width > MAX_FRAME_SIZE || aabb.height > MAX_FRAME_SIZE) {
        addError("location.aabb is too large (", MAX_FRAME_SIZE, " x ", MAX_FRAME_SIZE, ")");
    }
    if (origin.x > MAX_ORIGIN || origin.y > MAX_ORIGIN) {
        addError("location.origin is too large (max: ", MAX_ORIGIN, ", ", MAX_ORIGIN, ")");
    }
    if (aabb.size().contains(origin) == false) {
        addError("location.origin is not inside frame");
    }

    return valid;
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

/*
 * NOTE: ActionPoint::type is only tested if actionPointMapping is not empty,
 *       which allows utsi2utms to work without specifing a project file.
 */
inline bool Frame::validate(const ActionPointMapping& actionPointMapping, const Image& image, ErrorList& errorList) const
{
    bool valid = true;

    auto addError = [&](const auto... msg) {
        errorList.addError(frameError(*this, msg...));
        valid = false;
    };

    if (!name.isValid()) {
        addError("Missing name");
    }
    if (objects.size() > MAX_FRAME_OBJECTS) {
        addError("Too many frame objects");
    }
    if (actionPoints.size() > MAX_ACTION_POINTS) {
        addError("Too many action points");
    }
    if (entityHitboxes.size() > MAX_ENTITY_HITBOXES) {
        addError("Too many entity hitboxes");
    }

    valid &= location.validate(errorList, *this);

    if (image.size().contains(location.aabb) == false) {
        addError("Frame not inside image");
    }

    if (valid == false) {
        return false;
    }

    const usize frameSize = location.aabb.size();

    for (auto [i, obj] : const_enumerate(objects)) {
        if (frameSize.contains(obj.location, obj.sizePx()) == false) {
            errorList.addError(frameObjectError(*this, i, "Frame Object not inside frame"));
            valid = false;
        }
    }

    for (auto [i, ap] : const_enumerate(actionPoints)) {
        if (frameSize.contains(ap.location) == false) {
            errorList.addError(actionPointError(*this, i, "location not inside frame"));
            valid = false;
        }

        if (not actionPointMapping.empty()) {
            if (actionPointMapping.find(ap.type) == actionPointMapping.end()) {
                errorList.addError(actionPointError(*this, i, "Unknown action point type ", ap.type));
                valid = false;
            }
        }
    }

    for (auto [i, eh] : const_enumerate(entityHitboxes)) {
        if (eh.aabb.width == 0 || eh.aabb.height == 0) {
            errorList.addError(entityHitboxError(*this, i, "aabb has no size"));
            valid = false;
        }

        if (frameSize.contains(eh.aabb) == false) {
            errorList.addError(entityHitboxError(*this, i, "aabb not inside frame"));
            valid = false;
        }
    }

    if (solid) {
        if (!tileHitbox.contains(location.origin)) {
            addError("Frame origin must be inside the tile hitbox");
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

bool FrameSet::validate(const ActionPointMapping& actionPointMapping, ErrorList& errorList) const
{
    bool valid = true;
    auto addError = [&](std::string&& msg) {
        errorList.addErrorString(msg);
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
    valid &= grid.validate(errorList);

    if (valid == false) {
        return false;
    }

    const auto image = ImageCache::loadPngImage(imageFilename);
    assert(image);
    if (image->empty()) {
        addError(image->errorString());
        return false;
    }

    if (transparentColorValid(*image) == false) {
        addError("Transparent color is invalid");
    }

    if (palette.usesUserSuppliedPalette()) {
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

    valid &= validateNamesUnique(frames, "frame", errorList);
    valid &= validateNamesUnique(animations, "animation", errorList);

    for (auto& frame : frames) {
        valid &= frame.validate(actionPointMapping, *image, errorList);
    }

    for (auto& ani : animations) {
        valid &= ani.validate(*this, errorList);
    }

    return valid;
}

// This function exists to allows utsi2utms to work without specifing a project file
bool FrameSet::validate(ErrorList& errorList) const
{
    return validate(ActionPointMapping{}, errorList);
}

usize FrameSet::minimumFrameGridSize() const
{
    usize limit = usize(MIN_FRAME_SIZE, MIN_FRAME_SIZE);

    for (const Frame& frame : frames) {
        if (frame.location.useGridLocation) {
            limit = limit.expand(frame.minimumViableSize());
        }
    }

    return limit;
}

bool FrameSet::transparentColorValid(const Image& image) const
{
    if (transparentColor.alpha == 0xff) {
        return true;
    }
    if (transparentColor.alpha == 0) {
        bool hasTransparent = std::any_of(image.data(), image.data() + image.dataSize(),
                                          [&](const rgba& c) { return c.alpha == 0; });

        return hasTransparent && transparentColor == rgba(0, 0, 0, 0);
    }
    else {
        return false;
    }
}

void FrameSet::updateFrameLocations()
{
    for (Frame& frame : frames) {
        frame.location.update(grid, frame);
    }
}

bool FrameSet::operator==(const FrameSet& o) const
{
    return name == o.name
           && tilesetType == o.tilesetType
           && exportOrder == o.exportOrder
           && imageFilename == o.imageFilename
           && palette == o.palette
           && grid == o.grid
           && transparentColor == o.transparentColor
           && frames == o.frames
           && animations == o.animations;
}
