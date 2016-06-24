#include "animation.h"
#include "abstractframeset.h"
#include "models/common/namechecks.h"
#include <cmath>

using namespace UnTech;
using namespace UnTech::MetaSpriteCommon;

/*
 * ANIMATION
 * =========
 */

const char* Animation::TYPE_NAME = "Animation";

Animation::Animation(AbstractFrameSet& parent)
    : _frameSet(parent)
    , _instructions(*this)
{
}

Animation::Animation(const Animation& animation, AbstractFrameSet& parent)
    : _frameSet(parent)
    , _instructions(*this)
{
    for (const auto& inst : animation._instructions) {
        _instructions.clone(inst);
    }
}

/*
 * Valid if:
 *  * has at least one instruction
 *  * instruction ends with a stop or a goto
 *  * all instructions are valid
 */
bool Animation::isValid() const
{
    if (_instructions.size() == 0) {
        return false;
    }

    if (_instructions.last().operation().isValidTerminator() == false) {
        return false;
    }

    for (unsigned i = 0; i < _instructions.size(); i++) {
        const auto& inst = _instructions.at(i);

        if (inst.isValid(i) == false) {
            return false;
        }
    }

    // ::TODO verify animations do not have an infinite loop::

    return true;
}

/*
 * ANIMATION INSTRUCTIONS
 * ======================
 */

const char* AnimationInstruction::TYPE_NAME = "Animation Instruction";

AnimationInstruction::AnimationInstruction(Animation& parent)
    : _animation(parent)
    , _operation()
    , _frame()
    , _parameter(0)
{
}

AnimationInstruction::AnimationInstruction(const AnimationInstruction& ani,
                                           Animation& parent)
    : _animation(parent)
    , _operation(ani._operation)
    , _frame(ani._frame)
    , _parameter(ani._parameter)
{
}

void AnimationInstruction::setParameter(int p)
{
    if (_operation.usesParameter()) {
        if (_operation.parameterUnsigned()) {
            if (p >= 0 && p < 256) {
                _parameter = p;
            }
        }
        else {
            // parameter is unsigned
            _parameter = p;
        }
    }
}

void AnimationInstruction::setFrame(const FrameReference& frame)
{
    if (_operation.usesFrame()) {
        _frame = frame;
    }
}

void AnimationInstruction::setGotoLabel(const std::string& label)
{
    if (_operation.usesGotoLabel() && isNameValid(label)) {
        _gotoLabel = label;
    }
}

/*
 * Valid if:
 *  * GOTO_OFFSET points to an instruction inside the animation
 *  * Animation nane exists in the frameset
 *  * Frame name exists in the frameset
 */
bool AnimationInstruction::isValid(unsigned position) const
{
    const unsigned MAX_BYTECODE_SIZE = 3;

    if (_operation == AnimationBytecode::Enum::GOTO_OFFSET) {
        const int p = _parameter;
        const unsigned aSize = _animation.instructions().size();

        if (p == 0 || p < (int)-position || p >= (int)(aSize - position)) {
            return false;
        }

        if ((unsigned)abs(p) > 256 / MAX_BYTECODE_SIZE) {
            // simple bounds checking, prevent overflow
            return false;
        }
    }

    if (_operation.usesGotoLabel()) {
        if (_animation.frameSet().animations().nameExists(_gotoLabel) == false) {
            return false;
        }
    }

    if (_operation.usesFrame()) {
        if (_animation.frameSet().containsFrameName(_frame.frameName) == false) {
            return false;
        }
    }

    return true;
}
