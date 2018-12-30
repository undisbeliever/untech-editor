/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framesetexportlist.h"

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

namespace MS = UnTech::MetaSprite::MetaSprite;

using ExportName = FrameSetExportOrder::ExportName;

static std::vector<AnimationListEntry>
processAnimations(const MS::FrameSet& frameSet, const std::vector<ExportName>& animations)
{
    std::vector<AnimationListEntry> ret;
    ret.reserve(animations.size());

    for (const auto& en : animations) {
        const Animation::Animation* ani = frameSet.animations.getPtr(en.name);

        if (ani) {
            ret.push_back({ ani, false, false });
        }
        else {
            bool success = false;

            for (const auto& alt : en.alternatives) {
                const Animation::Animation* altAni = frameSet.animations.getPtr(alt.name);
                if (altAni) {
                    ret.push_back({ altAni, alt.hFlip, alt.vFlip });

                    success = true;
                    break;
                }
            }
            assert(success);
        }
    }

    // Include the animations referenced in the nextAnimation field
    for (unsigned toTestIndex = 0; toTestIndex < ret.size(); toTestIndex++) {
        const auto& ani = ret[toTestIndex];

        if (ani.animation->oneShot == false && ani.animation->nextAnimation.isValid()) {
            const idstring& nextAnimation = ani.animation->nextAnimation;

            const Animation::Animation* a = frameSet.animations.getPtr(nextAnimation);
            assert(a);

            AnimationListEntry toAdd = { a, ani.hFlip, ani.vFlip };

            auto it = std::find(ret.begin(), ret.end(), toAdd);
            if (it == ret.end()) {
                ret.push_back(toAdd);
            }
        }
    }

    return ret;
}

static std::vector<FrameListEntry>
processStillFrames(const MS::FrameSet& frameSet,
                   const std::vector<ExportName>& stillFrames,
                   const std::vector<AnimationListEntry>& animations)
{
    std::vector<FrameListEntry> ret;
    ret.reserve(stillFrames.size());

    for (const auto& en : stillFrames) {
        const MS::Frame* frame = frameSet.frames.getPtr(en.name);

        if (frame) {
            ret.push_back({ frame, false, false });
        }
        else {
            bool success = false;

            for (const auto& alt : en.alternatives) {
                const MS::Frame* altFrame = frameSet.frames.getPtr(alt.name);

                if (altFrame) {
                    ret.push_back({ altFrame, alt.hFlip, alt.vFlip });

                    success = true;
                    break;
                }
            }
            assert(success);
        }
    }

    // Add frames from animation.
    // Ensure that the frames added are unique.

    for (const AnimationListEntry& ani : animations) {
        for (const auto& aFrame : ani.animation->frames) {
            const auto& frameRef = aFrame.frame;

            const MS::Frame* frame = frameSet.frames.getPtr(frameRef.name);
            assert(frame);

            FrameListEntry e = { frame,
                                 static_cast<bool>(frameRef.hFlip ^ ani.hFlip),
                                 static_cast<bool>(frameRef.vFlip ^ ani.vFlip) };

            auto it = std::find(ret.begin(), ret.end(), e);
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
    const auto animations = processAnimations(frameSet, exportOrder.animations);

    return {
        frameSet,
        exportOrder,
        std::move(animations),
        processStillFrames(frameSet, exportOrder.stillFrames, animations)
    };
}

}
}
}
