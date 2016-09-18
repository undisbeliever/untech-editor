#include "spriteimporter.h"
#include "models/common/file.h"
#include <algorithm>

using namespace UnTech;
using namespace UnTech::MetaSprite::SpriteImporter;

template <>
const std::string FrameObject::list_t::HUMAN_TYPE_NAME = "Frame Object";
template <>
const std::string ActionPoint::list_t::HUMAN_TYPE_NAME = "Action Point";
template <>
const std::string EntityHitbox::list_t::HUMAN_TYPE_NAME = "Entity Hitbox";

bool FrameSetGrid::isValid(const FrameSet& frameSet) const
{
    usize minSize = frameSet.minimumViableFrameSize();

    return frameSize.width >= minSize.width
           && frameSize.height >= minSize.height
           && frameSize.contains(origin);
}

urect FrameSetGrid::cell(unsigned x, unsigned y) const
{
    return urect(
        x * (frameSize.width + padding.width) + offset.x,
        y * (frameSize.height + padding.height) + offset.y,
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
    aabb.height = std::max(aabb.width, minSize.height);

    if (useGridOrigin) {
        origin = grid.origin;
    }
    origin.x = std::min(origin.x, aabb.height);
    origin.y = std::min(origin.y, aabb.height);
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

usize FrameSet::minimumViableFrameSize() const
{
    usize limit;

    for (const auto& it : frames) {
        limit = limit.expand(it.second.minimumViableSize());
    }

    return limit;
}

void FrameSet::updateFrameLocations()
{
    for (auto& it : frames) {
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
