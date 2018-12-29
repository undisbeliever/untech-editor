/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
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

    const auto& frameSet = exportList.frameSet();
    const auto& frames = exportList.frames();
    const auto& animations = exportList.animations();

    assert(aniEntry.animation != nullptr);
    const ANI::Animation& animation = *aniEntry.animation;

    assert(animation.frames.empty() == false);

    uint8_t nextAnimationId = 0xff;
    {
        if (animation.oneShot == false) {
            const ANI::Animation* a = &animation;

            if (animation.nextAnimation.isValid()) {
                a = frameSet.animations.getPtr(animation.nextAnimation);
                assert(a != nullptr);
            }

            nextAnimationId = indexOf_throw(animations, { a, aniEntry.hFlip, aniEntry.vFlip });
        }
    }

    std::vector<uint8_t> data;
    data.reserve(3 + animation.frames.size() * 2);

    data.push_back(animation.durationFormat.engineValue()); // durationFormat
    data.push_back(nextAnimationId);                        // nextAnimation
    data.push_back(animation.frames.size() * 2);            // frameTableSize

    for (const auto& aFrame : animation.frames) {
        const auto& frameRef = aFrame.frame;

        uint8_t frameId = indexOf_throw(frames, { frameSet.frames.getPtr(frameRef.name),
                                                  static_cast<bool>(frameRef.hFlip ^ aniEntry.hFlip),
                                                  static_cast<bool>(frameRef.vFlip ^ aniEntry.vFlip) });

        data.push_back(frameId);         // Frames.frameId
        data.push_back(aFrame.duration); // Frames.duration
    }

    return data;
}

std::vector<std::vector<uint8_t>> processAnimations(const FrameSetExportList& exportList, ErrorList& errorList)
{
    const size_t nAnimations = exportList.animations().size();

    std::vector<std::vector<uint8_t>> ret;
    ret.reserve(nAnimations);

    assert(nAnimations <= MAX_EXPORT_NAMES);

    for (const auto& ani : exportList.animations()) {
        try {
            ret.push_back(processAnimation(ani, exportList));
        }
        catch (const std::exception& ex) {
            errorList.addError(exportList.frameSet(), *ani.animation, ex.what());
        }
    }

    return ret;
}

RomOffsetPtr saveAnimations(const std::vector<std::vector<uint8_t>>& animations, CompiledRomData& out)
{
    std::vector<uint32_t> offsets;
    offsets.reserve(animations.size());

    for (const auto& aData : animations) {
        offsets.emplace_back(out.animationData.addData(aData).offset);
    }

    return out.animationList.getOrInsertTable(offsets);
}

}
}
}
