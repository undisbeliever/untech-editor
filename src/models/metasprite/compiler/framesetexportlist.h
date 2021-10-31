/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "../frameset-exportorder.h"
#include "../metasprite.h"
#include <algorithm>
#include <cstdint>
#include <vector>

namespace UnTech::MetaSprite::Compiler {

struct AnimationListEntry {
    const Animation::Animation* animation;
    const bool hFlip;
    const bool vFlip;

    bool operator==(const AnimationListEntry&) const = default;
};

struct FrameListEntry {
    const MetaSprite::Frame* frame;
    const bool hFlip;
    const bool vFlip;

    bool operator==(const FrameListEntry&) const = default;
};

struct FrameSetExportList {
    const MetaSprite::FrameSet& frameSet;
    const FrameSetExportOrder& exportOrder;
    const std::vector<AnimationListEntry> animations;
    const std::vector<FrameListEntry> frames;
};

// NOTE: Can return an invalid FrameSetExportList
FrameSetExportList buildExportList(const MetaSprite::FrameSet& frameSet,
                                   const FrameSetExportOrder& exportOrder);

}
