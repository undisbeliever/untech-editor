#include "spriteimporter.h"
#include "models/common/file.h"
#include "models/common/humantypename.h"
#include <algorithm>

using namespace UnTech;
using namespace UnTech::MetaSprite::SpriteImporter;

template <>
const std::string HumanTypeName<FrameSet>::value = "Frame Set";
template <>
const std::string HumanTypeName<FrameSetGrid>::value = "Frame Set Grid";
template <>
const std::string HumanTypeName<Frame>::value = "Frame";
template <>
const std::string HumanTypeName<FrameLocation>::value = "Frame Location";
template <>
const std::string HumanTypeName<FrameObject>::value = "Frame Object";
template <>
const std::string HumanTypeName<ActionPoint>::value = "Action Point";
template <>
const std::string HumanTypeName<EntityHitbox>::value = "Entity Hitbox";

/*
 * FRAME SET GRID
 * ==============
 */

bool FrameSetGrid::isValid(const FrameSet& frameSet) const
{
    usize minSize = frameSet.minimumFrameGridSize();

    return frameSize.width >= minSize.width
           && frameSize.height >= minSize.height
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
           && aabb.size().contains(origin);
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

usize Frame::minimumViableSize() const
{
    usize limit = usize(MIN_FRAME_SIZE, MIN_FRAME_SIZE);

    limit = limit.expand(location.origin);

    if (solid) {
        limit = limit.expand(tileHitbox);
    }

    for (const auto& obj : objects) {
        limit = limit.expand(obj.bottomLeft());
    }

    for (const auto& ap : actionPoints) {
        limit = limit.expand(ap.location);
    }

    for (const auto& eh : entityHitboxes) {
        limit = limit.expand(eh.aabb);
    }

    return limit;
}

/*
 * FRAME SET
 * =========
 */

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

void FrameSet::loadImage(const std::string filename)
{
    imageFilename = File::fullPath(filename);
    reloadImage();
}

bool FrameSet::reloadImage()
{
    image.erase();

    bool ret = image.loadPngImage(imageFilename);

    if (ret && !transparentColorValid()) {
        transparentColor = image.getPixel(0, 0);
    }

    return ret;
}
