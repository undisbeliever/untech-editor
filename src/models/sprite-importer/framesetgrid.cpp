#include "framesetgrid.h"
#include "frameset.h"
#include "frame.h"
#include <algorithm>

using namespace UnTech;
using namespace UnTech::SpriteImporter;

FrameSetGrid::FrameSetGrid(FrameSet& frameSet)
    : _frameSet(frameSet)
    , _frameSize(Frame::MIN_SIZE)
    , _offset()
    , _padding()
    , _origin()
{
}

void FrameSetGrid::setFrameSize(usize frameSize)
{
    if (_frameSize != frameSize) {
        // determine the minimum size of a frame
        usize limit = Frame::MIN_SIZE;

        for (const auto f : _frameSet.frames()) {
            const auto& frame = f.second;

            if (frame->useGridLocation()) {
                limit.expand(frame->minimumViableSize());
            }
        }

        usize newSize = limit.expand(frameSize);

        if (_frameSize != newSize) {
            _frameSize = newSize;

            for (const auto f : _frameSet.frames()) {
                f.second->recalculateLocation();
            }
        }
    }
}

void FrameSetGrid::setOffset(upoint offset)
{
    if (_offset != offset) {
        _offset = offset;

        for (const auto f : _frameSet.frames()) {
            f.second->recalculateLocation();
        }
    }
}

void FrameSetGrid::setPadding(usize padding)
{
    if (_padding != padding) {
        _padding = padding;

        for (const auto f : _frameSet.frames()) {
            f.second->recalculateLocation();
        }
    }
}

void FrameSetGrid::setOrigin(upoint origin)
{
    upoint newOrigin = _frameSize.clip(origin);

    if (_origin != newOrigin) {
        _origin = newOrigin;

        for (const auto f : _frameSet.frames()) {
            f.second->recalculateOrigin();
        }
    }
}
