#include "framesetexportlist.h"
#include "models/metasprite-common/framesetexportorder.h"

using namespace UnTech::MetaSpriteCompiler;
namespace MS = UnTech::MetaSprite;
namespace MSC = UnTech::MetaSpriteCommon;

FrameSetExportList::FrameSetExportList(const MetaSprite::FrameSet& frameSet)
    : _frameSet(frameSet)
    , _animations()
    , _frames()
{
    if (frameSet.exportOrderDocument() == nullptr) {
        throw std::runtime_error("Missing MetaSprite Export Order Document");
    }

    buildAnimationList();
    buildFrameList();
}

inline void FrameSetExportList::buildAnimationList()
{
    assert(_frameSet.exportOrderDocument() != nullptr);
    const auto& exportOrder = _frameSet.exportOrderDocument()->exportOrder();

    _animations.reserve(exportOrder.animations().size());

    for (const auto& sfIt : exportOrder.animations()) {
        const std::string& aname = sfIt.first;

        MSC::Animation* ani = _frameSet.animations().getPtr(aname);

        if (ani != nullptr) {
            _animations.push_back({ ani, false, false });
        }
        else {
            bool success = false;

            for (const auto& alt : sfIt.second.alternativeNames()) {
                MSC::Animation* altAni = _frameSet.animations().getPtr(alt.name());

                if (altAni != nullptr) {
                    _animations.push_back({ altAni, alt.hFlip(), alt.vFlip() });

                    success = true;
                    break;
                }
            }

            if (success == false) {
                throw std::runtime_error("Cannot find animation " + aname);
            }
        }
    }
}

inline void FrameSetExportList::buildFrameList()
{
    assert(_frameSet.exportOrderDocument() != nullptr);
    const auto& exportOrder = _frameSet.exportOrderDocument()->exportOrder();

    _frames.reserve(exportOrder.stillFrames().size());

    for (const auto& sfIt : exportOrder.stillFrames()) {
        const std::string& fname = sfIt.first;

        MS::Frame* f = _frameSet.frames().getPtr(fname);

        if (f != nullptr) {
            _frames.push_back({ f, false, false });
        }
        else {
            bool success = false;

            for (const auto& alt : sfIt.second.alternativeNames()) {
                MS::Frame* af = _frameSet.frames().getPtr(alt.name());

                if (af != nullptr) {
                    _frames.push_back({ af, alt.hFlip(), alt.vFlip() });

                    success = true;
                    break;
                }
            }

            if (success == false) {
                throw std::runtime_error("Cannot find frame " + fname);
            }
        }
    }

    // Add frames from animation.
    // Ensure that the frames added are unique.

    assert(_animations.size() != 0 || exportOrder.animations().size() == 0);

    for (const AnimationListEntry& ani : _animations) {
        for (const auto& inst : ani.animation->instructions()) {
            const auto& op = inst.operation();
            if (op.usesFrame()) {
                const std::string& fname = inst.frame().frameName;

                FrameListEntry e = { _frameSet.frames().getPtr(fname),
                                     static_cast<bool>(inst.frame().hFlip ^ ani.hFlip),
                                     static_cast<bool>(inst.frame().vFlip ^ ani.vFlip) };

                if (e.frame == nullptr) {
                    throw std::runtime_error("Cannot find frame " + fname);
                }

                auto it = std::find(_frames.begin(), _frames.end(), e);
                if (it == _frames.end()) {
                    // new entry
                    _frames.push_back(e);
                }
            }
        }
    }
}
