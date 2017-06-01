/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framesetexportlist.h"

using namespace UnTech::MetaSprite;
using namespace UnTech::MetaSprite::Compiler;

namespace MS = UnTech::MetaSprite::MetaSprite;

FrameSetExportList::FrameSetExportList(const MetaSprite::FrameSet& frameSet)
    : _frameSet(frameSet)
    , _animations()
    , _frames()
{
    if (frameSet.exportOrder == nullptr) {
        throw std::runtime_error("Missing MetaSprite Export Order Document");
    }

    buildAnimationList();
    buildFrameList();
}

inline void FrameSetExportList::buildAnimationList()
{
    assert(_frameSet.exportOrder != nullptr);
    const auto& exportOrder = _frameSet.exportOrder;

    _animations.reserve(exportOrder->animations.size());

    for (const auto& en : exportOrder->animations) {
        const Animation::Animation* ani = _frameSet.animations.getPtr(en.name);

        if (ani) {
            _animations.push_back({ ani, false, false });
        }
        else {
            bool success = false;

            for (const auto& alt : en.alternatives) {
                const Animation::Animation* altAni = _frameSet.animations.getPtr(alt.name);
                if (altAni) {
                    _animations.push_back({ altAni, alt.hFlip, alt.vFlip });

                    success = true;
                    break;
                }
            }

            if (success == false) {
                throw std::runtime_error("Cannot find animation " + en.name);
            }
        }
    }

    // Include the animations referenced in the nextAnimation field
    for (unsigned toTestIndex = 0; toTestIndex < _animations.size(); toTestIndex++) {
        const auto& ani = _animations[toTestIndex];

        if (ani.animation->oneShot == false && ani.animation->nextAnimation.isValid()) {
            const idstring& nextAnimation = ani.animation->nextAnimation;

            const Animation::Animation* a = _frameSet.animations.getPtr(nextAnimation);
            if (a == nullptr) {
                throw std::runtime_error("Cannot find animation " + nextAnimation);
            }

            AnimationListEntry toAdd = { a, ani.hFlip, ani.vFlip };

            auto it = std::find(_animations.begin(), _animations.end(), toAdd);
            if (it == _animations.end()) {
                _animations.push_back(toAdd);
            }
        }
    }
}

inline void FrameSetExportList::buildFrameList()
{
    assert(_frameSet.exportOrder != nullptr);
    const auto& exportOrder = _frameSet.exportOrder;

    _frames.reserve(exportOrder->stillFrames.size());

    for (const auto& en : exportOrder->stillFrames) {
        const MS::Frame* frame = _frameSet.frames.getPtr(en.name);

        if (frame) {
            _frames.push_back({ frame, false, false });
        }
        else {
            bool success = false;

            for (const auto& alt : en.alternatives) {
                const MS::Frame* altFrame = _frameSet.frames.getPtr(alt.name);

                if (altFrame) {
                    _frames.push_back({ altFrame, alt.hFlip, alt.vFlip });

                    success = true;
                    break;
                }
            }

            if (success == false) {
                throw std::runtime_error("Cannot find frame " + en.name);
            }
        }
    }

    // Add frames from animation.
    // Ensure that the frames added are unique.

    assert(_animations.size() != 0 || exportOrder->animations.size() == 0);

    for (const AnimationListEntry& ani : _animations) {
        for (const auto& aFrame : ani.animation->frames) {
            const auto& frameRef = aFrame.frame;

            const MS::Frame* frame = _frameSet.frames.getPtr(frameRef.name);
            if (frame == nullptr) {
                throw std::runtime_error("Cannot find frame " + frameRef.name);
            }

            FrameListEntry e = { frame,
                                 static_cast<bool>(frameRef.hFlip ^ ani.hFlip),
                                 static_cast<bool>(frameRef.vFlip ^ ani.vFlip) };

            auto it = std::find(_frames.begin(), _frames.end(), e);
            if (it == _frames.end()) {
                // new entry
                _frames.push_back(e);
            }
        }
    }
}
