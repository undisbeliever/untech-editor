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
#include "models/metasprite/animation/animation.hpp"
#include <algorithm>

namespace UnTech::MetaSprite::SpriteImporter {

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

static bool validate(const FrameSetGrid& input, ErrorList& errorList)
{
    bool valid = true;
    auto addError = [&](const auto&... msg) {
        errorList.addErrorString(msg...);
        valid = false;
    };

    if (input.frameSize.width == 0 || input.frameSize.height == 0) {
        addError("grid.frameSize has no size");
    }
    if (input.frameSize.width > MAX_FRAME_SIZE || input.frameSize.height > MAX_FRAME_SIZE) {
        addError("grid.frameSize is too large (max: ", MAX_FRAME_SIZE, " x ", MAX_FRAME_SIZE, ")");
    }
    if (input.origin.x > MAX_ORIGIN || input.origin.y > MAX_ORIGIN) {
        addError("grid.origin is too large (max: ", MAX_ORIGIN, ", ", MAX_ORIGIN, ")");
    }
    if (input.frameSize.contains(input.origin) == false) {
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

static bool validate(const FrameLocation& input, const Frame& frame, const unsigned frameIndex, ErrorList& errorList)
{
    bool valid = true;
    auto addError = [&](const auto&... msg) {
        errorList.addError(frameError(frame, frameIndex, msg...));
        valid = false;
    };

    if (input.aabb.width == 0 || input.aabb.height == 0) {
        addError("FrameLocation aabb has no size");
    }
    if (input.aabb.width > MAX_FRAME_SIZE || input.aabb.height > MAX_FRAME_SIZE) {
        addError("location.aabb is too large (", MAX_FRAME_SIZE, " x ", MAX_FRAME_SIZE, ")");
    }
    if (input.origin.x > MAX_ORIGIN || input.origin.y > MAX_ORIGIN) {
        addError("location.origin is too large (max: ", MAX_ORIGIN, ", ", MAX_ORIGIN, ")");
    }
    if (input.aabb.size().contains(input.origin) == false) {
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
static bool validate(const Frame& input, const unsigned frameIndex, const Image& image, const ActionPointMapping& actionPointMapping,
                     ErrorList& errorList)
{
    bool valid = true;

    auto addError = [&](const auto... msg) {
        errorList.addError(frameError(input, frameIndex, msg...));
        valid = false;
    };

    if (!input.name.isValid()) {
        addError("Missing name");
    }
    if (input.objects.size() > MAX_FRAME_OBJECTS) {
        addError("Too many frame objects");
    }
    if (input.actionPoints.size() > MAX_ACTION_POINTS) {
        addError("Too many action points");
    }
    if (input.entityHitboxes.size() > MAX_ENTITY_HITBOXES) {
        addError("Too many entity hitboxes");
    }

    valid &= validate(input.location, input, frameIndex, errorList);

    if (image.size().contains(input.location.aabb) == false) {
        addError("Frame not inside image");
    }

    if (valid == false) {
        return false;
    }

    const usize frameSize = input.location.aabb.size();

    for (auto [i, obj] : const_enumerate(input.objects)) {
        if (frameSize.contains(obj.location, obj.sizePx()) == false) {
            errorList.addError(frameObjectError(input, frameIndex, i, "Frame Object not inside frame"));
            valid = false;
        }
    }

    for (auto [i, ap] : const_enumerate(input.actionPoints)) {
        if (frameSize.contains(ap.location) == false) {
            errorList.addError(actionPointError(input, frameIndex, i, "location not inside frame"));
            valid = false;
        }

        if (not actionPointMapping.empty()) {
            if (actionPointMapping.find(ap.type) == actionPointMapping.end()) {
                errorList.addError(actionPointError(input, frameIndex, i, "Unknown action point type ", ap.type));
                valid = false;
            }
        }
    }

    for (auto [i, eh] : const_enumerate(input.entityHitboxes)) {
        if (eh.aabb.width == 0 || eh.aabb.height == 0) {
            errorList.addError(entityHitboxError(input, frameIndex, i, "aabb has no size"));
            valid = false;
        }

        if (frameSize.contains(eh.aabb) == false) {
            errorList.addError(entityHitboxError(input, frameIndex, i, "aabb not inside frame"));
            valid = false;
        }
    }

    if (input.solid) {
        if (!input.tileHitbox.contains(input.location.origin)) {
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

static bool isTransparentColorValid(const FrameSet& input, const Image& image)
{
    if (input.transparentColor.alpha == 0xff) {
        return true;
    }
    if (input.transparentColor.alpha == 0) {
        bool hasTransparent = std::any_of(image.begin(), image.end(),
                                          [&](const rgba& c) { return c.alpha == 0; });

        return hasTransparent && input.transparentColor == rgba(0, 0, 0, 0);
    }
    else {
        return false;
    }
}

static bool validate(const FrameSet& input, const ActionPointMapping& actionPointMapping, ErrorList& errorList)
{
    bool valid = true;
    auto addError = [&](std::string&& msg) {
        errorList.addErrorString(std::move(msg));
        valid = false;
    };

    // Validate FrameSet

    if (input.name.isValid() == false) {
        addError("Missing name");
    }
    if (input.exportOrder.isValid() == false) {
        addError("Missing exportOrder");
    }
    if (input.frames.size() == 0) {
        addError("No Frames");
    }
    if (input.imageFilename.empty()) {
        addError("No Image");
    }
    valid &= validate(input.grid, errorList);

    if (valid == false) {
        return false;
    }

    const auto image = ImageCache::loadPngImage(input.imageFilename);
    assert(image);
    if (image->empty()) {
        addError(image->errorString());
        return false;
    }

    if (isTransparentColorValid(input, *image) == false) {
        addError("Transparent color is invalid");
    }

    if (input.palette.usesUserSuppliedPalette()) {
        auto imgSize = image->size();
        auto palSize = input.palette.paletteSize();

        if (input.palette.nPalettes > MAX_PALETTES) {
            addError("Too many palettes");
        }

        if (palSize.width > imgSize.width
            || palSize.height > imgSize.height) {

            addError("Palette outside image");
        }
    }

    if (input.animations.size() > MAX_ANIMATION_FRAMES) {
        addError("Too many animations in frameSet");
    }

    valid &= validateNamesUnique(input.frames, "frame", [&](unsigned i, auto... msg) {
        errorList.addError(std::make_unique<MetaSpriteError>(MsErrorType::FRAME, i, stringBuilder(msg...)));
    });
    valid &= validateNamesUnique(input.animations, "animation", [&](unsigned i, auto... msg) {
        errorList.addError(std::make_unique<MetaSpriteError>(MsErrorType::ANIMATION, i, stringBuilder(msg...)));
    });

    for (auto [i, frame] : enumerate(input.frames)) {
        valid &= validate(frame, i, *image, actionPointMapping, errorList);
    }

    for (auto [i, ani] : enumerate(input.animations)) {
        valid &= validate(ani, i, input, errorList);
    }

    return valid;
}

// This function exists to allows utsi2utms to work without specifing a project file
bool validate(const FrameSet& frameSet, ErrorList& errorList)
{
    return validate(frameSet, ActionPointMapping{}, errorList);
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

}
