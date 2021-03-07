/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationcompiler.h"
#include "compiler.h"
#include <algorithm>
#include <climits>

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

namespace MS = UnTech::MetaSprite::MetaSprite;
namespace ANI = UnTech::MetaSprite::Animation;

template <typename T>
static inline auto indexOf_throw(const std::vector<T>& vector, const T& item)
{
    auto it = std::find(vector.begin(), vector.end(), item);

    if (it == vector.end()) {
        throw std::out_of_range("item not found in vector");
    }

    return std::distance(vector.begin(), it);
}

static std::vector<uint8_t> processAnimation(const AnimationListEntry& aniEntry,
                                             const FrameSetExportList& exportList)
{
    using namespace UnTech;

    const auto& frameSet = exportList.frameSet;
    const auto& frames = exportList.frames;
    const auto& animations = exportList.animations;

    assert(aniEntry.animation != nullptr);
    const ANI::Animation& animation = *aniEntry.animation;

    assert(animation.frames.empty() == false);

    uint8_t nextAnimationId = 0xff;
    {
        if (animation.oneShot == false) {
            const ANI::Animation* nextAnimation = &animation;
            if (animation.nextAnimation.isValid()) {
                nextAnimation = &frameSet.animations.find(animation.nextAnimation).value();
            }

            nextAnimationId = indexOf_throw(animations, { nextAnimation, aniEntry.hFlip, aniEntry.vFlip });
        }
    }

    std::vector<uint8_t> data;
    data.reserve(3 + animation.frames.size() * 2);

    data.push_back(nextAnimationId);                        // nextAnimation
    data.push_back(animation.durationFormat.engineValue()); // durationFormat
    data.push_back(animation.frames.size() * 2);            // frameTableSize

    for (const auto& aFrame : animation.frames) {
        const auto& frameRef = aFrame.frame;

        uint8_t frameId = indexOf_throw(frames, { &*frameSet.frames.find(frameRef.name),
                                                  static_cast<bool>(frameRef.hFlip ^ aniEntry.hFlip),
                                                  static_cast<bool>(frameRef.vFlip ^ aniEntry.vFlip) });

        data.push_back(frameId);         // Frames.frameId
        data.push_back(aFrame.duration); // Frames.duration
    }

    return data;
}

std::vector<std::vector<uint8_t>> processAnimations(const FrameSetExportList& exportList)
{
    const size_t nAnimations = exportList.animations.size();

    std::vector<std::vector<uint8_t>> ret;
    ret.reserve(nAnimations);

    assert(nAnimations <= MAX_EXPORT_NAMES);

    for (const auto& ani : exportList.animations) {
        ret.push_back(processAnimation(ani, exportList));
    }

    return ret;
}

uint16_t saveAnimations(const std::vector<std::vector<uint8_t>>& animations, CompiledRomData& out)
{
    if (animations.size() == 0) {
        return 0;
    }

    DataBlock table(animations.size() * 2);

    for (const auto& aData : animations) {
        uint16_t index = out.animationData.addData_Index(aData);
        table.addWord(index);
    }

    return out.animationList.addData_Index(table.data());
}

}
}
}
