/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationcompiler.h"
#include <algorithm>
#include <climits>
#include <set>

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

inline uint32_t
AnimationCompiler::processAnimation(const AnimationListEntry& aniEntry,
                                    const MS::FrameSet& frameSet,
                                    const std::map<const FrameListEntry, unsigned>& frameMap,
                                    const std::map<const AnimationListEntry, unsigned>& animationMap)
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

            nextAnimationId = animationMap.at({ a, aniEntry.hFlip, aniEntry.vFlip });
        }
    }

    std::vector<uint8_t> data;
    data.reserve(3 + animation.frames.size() * 2);

    data.push_back(animation.durationFormat.engineValue()); // durationFormat
    data.push_back(nextAnimationId);                        // nextAnimation
    data.push_back(animation.frames.size() * 2);            // frameTableSize

    for (const auto& aFrame : animation.frames) {
        const auto& frameRef = aFrame.frame;

        uint8_t frameId = frameMap.at({ frameSet.frames.getPtr(frameRef.name),
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
    const auto& animationList = exportList.animations();
    const auto& frameList = exportList.frames();

    assert(animationList.size() <= MAX_EXPORT_NAMES);

    std::map<const FrameListEntry, unsigned> frameMap;
    for (unsigned i = 0; i < frameList.size(); i++) {
        frameMap[frameList[i]] = i;
    }

    std::map<const AnimationListEntry, unsigned> animationMap;
    for (unsigned i = 0; i < animationList.size(); i++) {
        animationMap[animationList[i]] = i;
    }

    std::vector<uint32_t> animationOffsets;
    animationOffsets.reserve(animationList.size());

    for (const auto& ani : animationList) {
        assert(ani.animation != nullptr);
        uint32_t ao = ~0;

        bool valid = ani.animation->isValid(exportList.frameSet());
        if (valid) {
            try {
                ao = processAnimation(ani, exportList.frameSet(),
                                      frameMap, animationMap);
            }
            catch (const std::exception& ex) {
                _errorList.addError(exportList.frameSet(), *ani.animation, ex.what());
            }
        }
        else {
            _errorList.addError(exportList.frameSet(), *ani.animation, "Invalid Animation");
        }

        animationOffsets.push_back(ao);
    }

    return _animationList.getOrInsertTable(animationOffsets);
}
