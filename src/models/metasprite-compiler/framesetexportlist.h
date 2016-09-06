#pragma once

#include "models/metasprite.h"
#include <algorithm>
#include <cstdint>
#include <vector>

namespace UnTech {
namespace MetaSpriteCompiler {

struct AnimationListEntry {
    const MetaSpriteCommon::Animation* animation;
    const bool hFlip;
    const bool vFlip;

    inline bool operator==(const AnimationListEntry& o) const
    {
        return animation == o.animation && hFlip == o.hFlip && vFlip == o.vFlip;
    }

    inline bool operator<(const AnimationListEntry& o) const
    {
        return std::tie(animation, hFlip, vFlip) < std::tie(o.animation, o.hFlip, o.vFlip);
    }
};

struct FrameListEntry {
    const MetaSprite::Frame* frame;
    const bool hFlip;
    const bool vFlip;

    inline bool operator==(const FrameListEntry& o) const
    {
        return frame == o.frame && hFlip == o.hFlip && vFlip == o.vFlip;
    }

    inline bool operator<(const FrameListEntry& o) const
    {
        return std::tie(frame, hFlip, vFlip) < std::tie(o.frame, o.hFlip, o.vFlip);
    }
};

class FrameSetExportList {
public:
    FrameSetExportList(const MetaSprite::FrameSet& frameSet);
    FrameSetExportList(const FrameSetExportList&) = delete;

    const MetaSprite::FrameSet& frameSet() const { return _frameSet; }
    const std::vector<AnimationListEntry>& animations() const { return _animations; }
    const std::vector<FrameListEntry>& frames() const { return _frames; }

private:
    void buildAnimationList();
    void buildFrameList();

private:
    const MetaSprite::FrameSet& _frameSet;
    std::vector<AnimationListEntry> _animations;
    std::vector<FrameListEntry> _frames;
};
}
}
