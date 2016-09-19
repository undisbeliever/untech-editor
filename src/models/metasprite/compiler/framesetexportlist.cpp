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
    typedef Animation::Bytecode::Enum BC;

    assert(_frameSet.exportOrder != nullptr);
    const auto& exportOrder = _frameSet.exportOrder;

    _animations.reserve(exportOrder->animations.size());

    for (const auto& en : exportOrder->animations) {
        const auto aniIt = _frameSet.animations.find(en.name);

        if (aniIt != _frameSet.animations.end()) {
            const Animation::Animation* ani = &aniIt->second;
            _animations.push_back({ ani, false, false });
        }
        else {
            bool success = false;

            for (const auto& alt : en.alternatives) {
                const auto altIt = _frameSet.animations.find(alt.name);

                if (altIt != _frameSet.animations.end()) {
                    const Animation::Animation* altAni = &altIt->second;
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

    // Include the animations referenced in GOTO_ANIMATION bytecode
    for (unsigned toTestIndex = 0; toTestIndex < _animations.size(); toTestIndex++) {
        const auto& toTest = _animations[toTestIndex];

        for (const auto& inst : toTest.animation->instructions) {
            if (inst.operation == BC::GOTO_ANIMATION) {
                const auto aIt = _frameSet.animations.find(inst.gotoLabel);

                if (aIt == _frameSet.animations.end()) {
                    throw std::runtime_error("Cannot find animation " + inst.gotoLabel);
                }
                const Animation::Animation* a = &aIt->second;

                auto it = std::find(_animations.begin(), _animations.end(),
                                    AnimationListEntry({ a, toTest.hFlip, toTest.vFlip }));
                if (it == _animations.end()) {
                    _animations.push_back({ a, toTest.hFlip, toTest.vFlip });
                }
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
        const auto fIt = _frameSet.frames.find(en.name);

        if (fIt != _frameSet.frames.end()) {
            const MS::Frame* f = &fIt->second;
            _frames.push_back({ f, false, false });
        }
        else {
            bool success = false;

            for (const auto& alt : en.alternatives) {
                const auto afIt = _frameSet.frames.find(alt.name);

                if (afIt != _frameSet.frames.end()) {
                    const MS::Frame* af = &afIt->second;
                    _frames.push_back({ af, alt.hFlip, alt.vFlip });

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
        for (const auto& inst : ani.animation->instructions) {
            const auto& op = inst.operation;
            if (op.usesFrame()) {
                auto fIt = _frameSet.frames.find(inst.frame.name);

                if (fIt == _frameSet.frames.end()) {
                    throw std::runtime_error("Cannot find frame " + inst.frame.name);
                }

                FrameListEntry e = { &fIt->second,
                                     static_cast<bool>(inst.frame.hFlip ^ ani.hFlip),
                                     static_cast<bool>(inst.frame.vFlip ^ ani.vFlip) };

                auto it = std::find(_frames.begin(), _frames.end(), e);
                if (it == _frames.end()) {
                    // new entry
                    _frames.push_back(e);
                }
            }
        }
    }
}
