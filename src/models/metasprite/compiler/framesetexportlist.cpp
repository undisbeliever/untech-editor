/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framesetexportlist.h"
#include "models/common/errorlist.h"

namespace UnTech::MetaSprite::Compiler {

namespace MS = UnTech::MetaSprite::MetaSprite;

using ExportName = FrameSetExportOrder::ExportName;

template <typename T>
static std::vector<ExportIndex> baseExportList(const NamedList<T>& fsList, const NamedList<ExportName>& exportList)
{
    std::vector<ExportIndex> ret;
    ret.reserve(exportList.size());

    for (const auto& en : exportList) {
        const auto aniIndex = fsList.indexOf(en.name);
        if (aniIndex) {
            ret.push_back({ unsigned(aniIndex.value()), false, false });
        }
        else {
            bool success = false;

            for (const auto& alt : en.alternatives) {
                const auto altIndex = fsList.indexOf(alt.name);
                if (altIndex) {
                    ret.push_back({ unsigned(altIndex.value()), alt.hFlip, alt.vFlip });

                    success = true;
                    break;
                }
            }
            assert(success);
        }
    }

    return ret;
}

static std::vector<ExportIndex>
processAnimations(const MS::FrameSet& frameSet, const NamedList<ExportName>& animations)
{
    auto ret = baseExportList(frameSet.animations, animations);

    // Include the animations referenced in the nextAnimation field
    // Must use an old-style for loop, ret is resized inside this loop.
    for (unsigned toTestIndex = 0; toTestIndex < ret.size(); toTestIndex++) {
        const auto& ani = ret[toTestIndex];
        const auto& animation = frameSet.animations.at(ani.fsIndex);

        if (animation.oneShot == false && animation.nextAnimation.isValid()) {
            const idstring& nextAnimation = animation.nextAnimation;

            const auto nextIndex = frameSet.animations.indexOf(nextAnimation);

            ExportIndex toAdd = { unsigned(nextIndex.value()), ani.hFlip, ani.vFlip };

            const auto it = std::find(ret.begin(), ret.end(), toAdd);
            if (it == ret.end()) {
                ret.push_back(toAdd);
            }
        }
    }

    return ret;
}

static std::vector<ExportIndex>
processStillFrames(const MS::FrameSet& frameSet,
                   const NamedList<ExportName>& stillFrames,
                   const std::vector<ExportIndex>& animations)
{
    auto ret = baseExportList(frameSet.frames, stillFrames);

    // Add frames from animation.
    // Ensure that the frames added are unique.

    for (const ExportIndex& ani : animations) {
        const auto& animation = frameSet.animations.at(ani.fsIndex);
        for (const auto& aFrame : animation.frames) {
            const auto& frameRef = aFrame.frame;

            const auto frameIndex = frameSet.frames.indexOf(frameRef.name);

            ExportIndex e = { unsigned(frameIndex.value()),
                              static_cast<bool>(frameRef.hFlip ^ ani.hFlip),
                              static_cast<bool>(frameRef.vFlip ^ ani.vFlip) };

            const auto it = std::find(ret.begin(), ret.end(), e);
            if (it == ret.end()) {
                // new entry
                ret.push_back(e);
            }
        }
    }

    return ret;
}

FrameSetExportList buildExportList(const MS::FrameSet& frameSet, const FrameSetExportOrder& exportOrder)
{
    auto animations = processAnimations(frameSet, exportOrder.animations);
    auto frames = processStillFrames(frameSet, exportOrder.stillFrames, animations);

    return {
        std::move(animations),
        std::move(frames),
    };
}

}
