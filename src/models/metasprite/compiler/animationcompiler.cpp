#include "animationcompiler.h"
#include <algorithm>
#include <climits>
#include <set>

using namespace UnTech::MetaSprite::Compiler;
namespace MS = UnTech::MetaSprite::MetaSprite;

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
    typedef Animation::Bytecode::Enum BC;

    assert(aniEntry.animation != nullptr);
    const Animation::Animation& animation = *aniEntry.animation;

    std::vector<uint8_t> data;
    data.reserve(animation.instructions.size() * 3);

    const auto& instructions = animation.instructions;
    for (auto instIt = instructions.begin(); instIt != instructions.end(); ++instIt) {
        const auto& inst = *instIt;

        data.push_back(inst.operation.engineValue());

        switch (inst.operation.value()) {
        case BC::GOTO_ANIMATION: {
            const Animation::Animation* a = frameSet.animations.getPtr(inst.gotoLabel);

            if (a == nullptr) {
                throw std::runtime_error("GOTO_ANIMATION: Cannot find animation " + inst.gotoLabel);
            }

            // Find animation Id
            auto it = animationMap.find({ a, aniEntry.hFlip, aniEntry.vFlip });
            if (it == animationMap.end()) {
                it = animationMap.find({ a, false, false });
            }
            if (it == animationMap.end()) {
                throw std::runtime_error("GOTO_ANIMATION: Cannot find animation " + inst.gotoLabel);
            }
            data.push_back(it->second);

            break;
        }

        case BC::GOTO_OFFSET: {
            // will always succeed because inst is valid.

            int p = inst.parameter;
            int offset = 0;

            if (p < 0) {
                assert(std::distance(instructions.begin(), instIt) >= -p);
                assert(instIt + (p + 1) != instructions.begin());

                for (auto gotoIt = instIt + p; gotoIt != instIt; ++gotoIt) {
                    offset -= (*gotoIt).operation.instructionSize();
                }
            }
            else {
                assert(std::distance(instIt, instructions.end()) >= p);
                assert(instIt + p != instructions.end());

                for (auto gotoIt = instIt + p; gotoIt != instIt; --gotoIt) {
                    offset += (*gotoIt).operation.instructionSize();
                }
            }

            if (!int_ms8_t::isValid(offset)) {
                throw std::runtime_error("GOTO_OFFSET: Offset outside range");
            }

            data.push_back(int_ms8_t(offset).romData());

            break;
        }

        case BC::SET_FRAME_AND_WAIT_FRAMES:
        case BC::SET_FRAME_AND_WAIT_TIME:
        case BC::SET_FRAME_AND_WAIT_XVECL:
        case BC::SET_FRAME_AND_WAIT_YVECL: {
            const MS::Frame* frame = frameSet.frames.getPtr(inst.frame.name);
            assert(frame != nullptr);

            FrameListEntry f = { frame,
                                 static_cast<bool>(inst.frame.hFlip ^ aniEntry.hFlip),
                                 static_cast<bool>(inst.frame.vFlip ^ aniEntry.vFlip) };

            data.push_back(frameMap.at(f));

            assert(inst.parameter >= 0 && inst.parameter < UINT8_MAX);
            data.push_back(inst.parameter);

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
