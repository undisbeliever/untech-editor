#include "animation.h"
#include "models/common/humantypename.h"
#include "models/metasprite/metasprite.h"
#include "models/metasprite/spriteimporter.h"

using namespace UnTech;
using namespace UnTech::MetaSprite::Animation;

namespace MS = UnTech::MetaSprite::MetaSprite;
namespace SI = UnTech::MetaSprite::SpriteImporter;

template <>
const std::string HumanTypeName<Animation>::value = "Animation";
template <>
const std::string HumanTypeName<Instruction>::value = "Animation Instruction";

// Instruction
// ===========
const unsigned MAX_BYTECODE_SIZE = 3;
static_assert(MetaSprite::MAX_ANIMATION_INSTRUCTIONS < 255 / MAX_BYTECODE_SIZE,
              "MAX_ANIMATION_INSTRUCTIONS too large");

template <class FrameSetT>
bool Instruction::_isValid(const FrameSetT& frameSet, const Animation& animation,
                           unsigned position) const
{
    if (operation == Bytecode::Enum::GOTO_OFFSET) {
        const int p = parameter;
        const unsigned aSize = animation.instructions.size();

        if ((unsigned)abs(p) >= MAX_ANIMATION_INSTRUCTIONS) {
            return false;
        }

        assert(position < MAX_ANIMATION_INSTRUCTIONS);
        if (p == 0 || p < (int)-position || p >= (int)(aSize - position)) {
            return false;
        }
    }

    if (operation.usesGotoLabel()) {
        if (frameSet.animations.contains(gotoLabel) == false) {
            return false;
        }
    }

    if (operation.usesFrame()) {
        if (frameSet.frames.contains(frame.name) == false) {
            return false;
        }
    }

    return true;
}

bool Instruction::isValid(const MS::FrameSet& frameSet, const Animation& animation,
                          unsigned position) const
{
    return _isValid(frameSet, animation, position);
}

bool Instruction::isValid(const SI::FrameSet& frameSet, const Animation& animation,
                          unsigned position) const
{
    return _isValid(frameSet, animation, position);
}

// Animation
// =========

template <class FrameSetT>
bool Animation::_isValid(const FrameSetT& frameSet) const
{
    if (instructions.size() == 0) {
        return false;
    }

    if (instructions.back().operation.isValidTerminator() == false) {
        return false;
    }

    for (unsigned i = 0; i < instructions.size(); i++) {
        const auto& inst = instructions.at(i);

        if (inst.isValid(frameSet, *this, i) == false) {
            return false;
        }
    }

    return true;
}

bool Animation::isValid(const MS::FrameSet& frameSet) const
{
    return _isValid(frameSet);
}

bool Animation::isValid(const SI::FrameSet& frameSet) const
{
    return _isValid(frameSet);
}
