/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "../metasprite.h"
#include "../project.h"
#include <algorithm>
#include <cstdint>
#include <vector>

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

struct AnimationListEntry {
    const Animation::Animation* animation;
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

struct FrameSetExportList {
    const MetaSprite::FrameSet& frameSet;
    const FrameSetExportOrder& exportOrder;
    const std::vector<AnimationListEntry> animations;
    const std::vector<FrameListEntry> frames;

    bool validate(ErrorList& err) const;
};

// NOTE: Can return an invalid FrameSetExportList
FrameSetExportList buildExportList(const MetaSprite::FrameSet& frameSet,
                                   const FrameSetExportOrder& exportOrder);

}
}
}
