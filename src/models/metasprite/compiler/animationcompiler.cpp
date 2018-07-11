/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationcompiler.h"
#include <algorithm>
#include <climits>

using namespace UnTech;
using namespace UnTech::MetaSprite::Compiler;
namespace MS = UnTech::MetaSprite::MetaSprite;
namespace ANI = UnTech::MetaSprite::Animation;

AnimationCompiler::AnimationCompiler(ErrorList& errorList)
    : _errorList(errorList)
    , _animationData("AD", "MS_AnimationData")
    , _animationList("AL", "MS_AnimationList", "AD")
{
}

void AnimationCompiler::writeToIncFile(std::ostream& out) const
{
    _animationData.writeToIncFile(out);
    _animationList.writeToIncFile(out);
}

template <typename T>
static inline auto indexOf_throw(const std::vector<T>& vector, const T& item)
{
    auto it = std::find(vector.begin(), vector.end(), item);

    if (it == vector.end()) {
        throw std::out_of_range("item not found in vector");
    }

    return std::distance(vector.begin(), it);
}

inline uint32_t
AnimationCompiler::processAnimation(const AnimationListEntry& aniEntry,
                                    const MS::FrameSet& frameSet,
                                    const std::vector<FrameListEntry>& frames,
                                    const std::vector<AnimationListEntry>& animations)
{
    using namespace UnTech;

    assert(aniEntry.animation != nullptr);
    const Animation::Animation& animation = *aniEntry.animation;

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

    return _animationData.addData(data).offset;
}

RomOffsetPtr
AnimationCompiler::process(const FrameSetExportList& exportList)
{
    const size_t nAnimations = exportList.animations().size();

    if (nAnimations > MAX_EXPORT_NAMES) {
        _errorList.addError("Too many animations in export order");
        return _animationList.addNull();
    }

    std::vector<uint32_t> animationOffsets;
    animationOffsets.reserve(nAnimations);

    for (const auto& ani : exportList.animations()) {
        assert(ani.animation != nullptr);
        uint32_t ao = ~0;

        assert(ani.animation->isValid(exportList.frameSet()));

        try {
            ao = processAnimation(ani, exportList.frameSet(),
                                  exportList.frames(), exportList.animations());
        }
        catch (const std::exception& ex) {
            _errorList.addError(exportList.frameSet(), *ani.animation, ex.what());
        }

        animationOffsets.push_back(ao);
    }

    return _animationList.getOrInsertTable(animationOffsets);
}
