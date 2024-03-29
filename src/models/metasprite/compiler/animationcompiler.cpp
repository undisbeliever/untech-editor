/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationcompiler.h"
#include "compiler.h"
#include <algorithm>
#include <climits>

namespace UnTech::MetaSprite::Compiler {

namespace MS = UnTech::MetaSprite::MetaSprite;
namespace ANI = UnTech::MetaSprite::Animation;

template <typename T>
static inline auto indexOf_throw(const std::vector<T>& vector, const T& item)
{
    auto it = std::find(vector.begin(), vector.end(), item);

    if (it == vector.end()) {
        throw out_of_range(u8"item not found in vector");
    }

    return std::distance(vector.begin(), it);
}

static std::vector<uint8_t> processAnimation(const ExportIndex& aniEntry,
                                             const FrameSetExportList& exportList,
                                             const MS::FrameSet& frameSet)
{
    const auto& frames = exportList.frames;
    const auto& animations = exportList.animations;

    const ANI::Animation& animation = frameSet.animations.at(aniEntry.fsIndex);

    assert(animation.frames.empty() == false);

    uint8_t nextAnimationId = 0xff;
    {
        if (animation.oneShot == false) {
            unsigned nextAnimation = aniEntry.fsIndex;
            if (animation.nextAnimation.isValid()) {
                const auto aIndex = frameSet.animations.indexOf(animation.nextAnimation);
                assert(aIndex);
                nextAnimation = aIndex.value();
            }

            nextAnimationId = indexOf_throw(animations, { nextAnimation, aniEntry.hFlip, aniEntry.vFlip });
        }
    }

    std::vector<uint8_t> data;
    data.reserve(3 + animation.frames.size() * 2);

    data.push_back(nextAnimationId);                       // nextAnimation
    data.push_back(engineValue(animation.durationFormat)); // durationFormat
    data.push_back(animation.frames.size() * 2);           // frameTableSize

    for (const auto& aFrame : animation.frames) {
        const auto& frameRef = aFrame.frame;

        const auto frameIndex = frameSet.frames.indexOf(frameRef.name);
        assert(frameIndex);

        uint8_t frameId = indexOf_throw(frames, { unsigned(frameIndex.value()),
                                                  static_cast<bool>(frameRef.hFlip ^ aniEntry.hFlip),
                                                  static_cast<bool>(frameRef.vFlip ^ aniEntry.vFlip) });

        data.push_back(frameId);         // Frames.frameId
        data.push_back(aFrame.duration); // Frames.duration
    }

    return data;
}

std::vector<std::vector<uint8_t>> processAnimations(const FrameSetExportList& exportList,
                                                    const MS::FrameSet& frameSet)
{
    std::vector<std::vector<uint8_t>> ret(exportList.animations.size());
    std::transform(exportList.animations.begin(), exportList.animations.end(),
                   ret.begin(),
                   [&](const auto& ani) { return processAnimation(ani, exportList, frameSet); });

    assert(ret.size() <= MAX_EXPORT_NAMES);

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
