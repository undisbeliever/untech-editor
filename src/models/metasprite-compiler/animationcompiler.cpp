#include "animationcompiler.h"
#include <algorithm>
#include <climits>
#include <set>

using namespace UnTech::MetaSpriteCompiler;
namespace MS = UnTech::MetaSprite;
namespace MSC = UnTech::MetaSpriteCommon;

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
    typedef MSC::AnimationBytecode::Enum BC;

    assert(aniEntry.animation != nullptr);
    const MSC::Animation& animation = *aniEntry.animation;

    std::vector<uint8_t> data;
    data.reserve(animation.instructions().size() * 3);

    const auto& instructions = animation.instructions();
    for (auto instIt = instructions.begin(); instIt != instructions.end(); ++instIt) {
        const auto& inst = *instIt;

        data.push_back(inst.operation().engineValue());

        switch (inst.operation().value()) {
        case BC::GOTO_ANIMATION: {
            const MSC::Animation* a = frameSet.animations().getPtr(inst.gotoLabel());

            // Find animation Id
            if (a == nullptr) {
                throw std::runtime_error("Cannot find animation " + inst.gotoLabel());
            }
            auto it = animationMap.find({ a, aniEntry.hFlip, aniEntry.vFlip });
            if (it == animationMap.end()) {
                it = animationMap.find({ a, false, false });
            }
            if (it == animationMap.end()) {
                throw std::runtime_error("Cannot find animation " + inst.gotoLabel());
            }
            data.push_back(it->second);

            break;
        }

        case BC::GOTO_OFFSET: {
            // will always succeed because inst is valid.

            int p = inst.parameter();
            UnTech::int_ms8_t offset = 0;

            if (p < 0) {
                assert(std::distance(instructions.begin(), instIt) >= -p);
                assert(instIt + (p + 1) != instructions.begin());

                for (auto gotoIt = instIt + p; gotoIt != instIt; ++gotoIt) {
                    offset -= (*gotoIt)->operation().instructionSize();
                }
            }
            else {
                assert(std::distance(instIt, instructions.end()) >= p);
                assert(instIt + p != instructions.end());

                for (auto gotoIt = instIt + p; gotoIt != instIt; --gotoIt) {
                    offset += (*gotoIt)->operation().instructionSize();
                }
            }

            data.push_back(offset.romData());

            break;
        }

        case BC::SET_FRAME_AND_WAIT_FRAMES:
        case BC::SET_FRAME_AND_WAIT_TIME:
        case BC::SET_FRAME_AND_WAIT_XVECL:
        case BC::SET_FRAME_AND_WAIT_YVECL: {
            const std::string& fname = inst.frame().frameName;

            FrameListEntry f = { frameSet.frames().getPtr(fname),
                                 static_cast<bool>(inst.frame().hFlip ^ aniEntry.hFlip),
                                 static_cast<bool>(inst.frame().vFlip ^ aniEntry.vFlip) };

            data.push_back(frameMap.at(f));

            assert(inst.parameter() >= 0 && inst.parameter() < 256);
            data.push_back(inst.parameter());

            break;
        }

        case BC::STOP:
        case BC::GOTO_START:
            break;
        }
    }

    return _animationData.addData(data).offset;
}

RomOffsetPtr
AnimationCompiler::process(const FrameSetExportList& exportList)
{
    const auto& animationList = exportList.animations();
    const auto& frameList = exportList.frames();

    if (animationList.size() >= 256) {
        throw std::runtime_error("Too many animations");
    }

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

        if (ani.animation->isValid()) {
            ao = processAnimation(ani, exportList.frameSet(),
                                  frameMap, animationMap);
        }
        else {
            _errorList.addError(*ani.animation, "animation is invalid");
        }

        animationOffsets.push_back(ao);
    }

    return _animationList.getOrInsertTable(animationOffsets);
}
