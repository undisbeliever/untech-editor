#include "animationbytecode.h"
#include <stdexcept>

using namespace UnTech::MetaSpriteCommon;

const std::map<AnimationBytecode::Enum, std::string> AnimationBytecode::enumMap = {
    { AnimationBytecode::Enum::STOP, "STOP" },
    { AnimationBytecode::Enum::GOTO_START, "GOTO_START" },
    { AnimationBytecode::Enum::GOTO_ANIMATION, "GOTO_ANIMATION" },
    { AnimationBytecode::Enum::GOTO_OFFSET, "GOTO_OFFSET" },
    { AnimationBytecode::Enum::SET_FRAME_AND_WAIT_FRAMES, "SET_FRAME_AND_WAIT_FRAMES" },
    { AnimationBytecode::Enum::SET_FRAME_AND_WAIT_TIME, "SET_FRAME_AND_WAIT_TIME" },
    { AnimationBytecode::Enum::SET_FRAME_AND_WAIT_XVECL, "SET_FRAME_AND_WAIT_XVECL" },
    { AnimationBytecode::Enum::SET_FRAME_AND_WAIT_YVECL, "SET_FRAME_AND_WAIT_YVECL" },
};

const std::map<std::string, AnimationBytecode::Enum> AnimationBytecode::stringMap = {
    { "STOP", AnimationBytecode::Enum::STOP },
    { "GOTO_START", AnimationBytecode::Enum::GOTO_START },
    { "GOTO_ANIMATION", AnimationBytecode::Enum::GOTO_ANIMATION },
    { "GOTO_OFFSET", AnimationBytecode::Enum::GOTO_OFFSET },
    { "SET_FRAME_AND_WAIT_FRAMES", AnimationBytecode::Enum::SET_FRAME_AND_WAIT_FRAMES },
    { "SET_FRAME_AND_WAIT_TIME", AnimationBytecode::Enum::SET_FRAME_AND_WAIT_TIME },
    { "SET_FRAME_AND_WAIT_XVECL", AnimationBytecode::Enum::SET_FRAME_AND_WAIT_XVECL },
    { "SET_FRAME_AND_WAIT_YVECL", AnimationBytecode::Enum::SET_FRAME_AND_WAIT_YVECL },
};

unsigned AnimationBytecode::instructionSize() const
{
    switch (_value) {
    default:
    case Enum::STOP:
    case Enum::GOTO_START:
        return 1;

    case Enum::GOTO_ANIMATION:
    case Enum::GOTO_OFFSET:
        return 2;

    case Enum::SET_FRAME_AND_WAIT_FRAMES:
    case Enum::SET_FRAME_AND_WAIT_TIME:
    case Enum::SET_FRAME_AND_WAIT_XVECL:
    case Enum::SET_FRAME_AND_WAIT_YVECL:
        return 3;
    }
}

bool AnimationBytecode::isValidTerminator() const
{
    switch (_value) {
    case Enum::STOP:
    case Enum::GOTO_START:
    case Enum::GOTO_ANIMATION:
    case Enum::GOTO_OFFSET:
        return true;

    default:
        return false;
    }
}

bool AnimationBytecode::usesFrame() const
{
    switch (_value) {
    case Enum::SET_FRAME_AND_WAIT_FRAMES:
    case Enum::SET_FRAME_AND_WAIT_TIME:
    case Enum::SET_FRAME_AND_WAIT_XVECL:
    case Enum::SET_FRAME_AND_WAIT_YVECL:
        return true;

    default:
        return false;
    }
}

bool AnimationBytecode::usesParameter() const
{
    switch (_value) {
    case Enum::GOTO_OFFSET:
    case Enum::SET_FRAME_AND_WAIT_FRAMES:
    case Enum::SET_FRAME_AND_WAIT_TIME:
    case Enum::SET_FRAME_AND_WAIT_XVECL:
    case Enum::SET_FRAME_AND_WAIT_YVECL:
        return true;

    default:
        return false;
    }
}

bool AnimationBytecode::parameterUnsigned() const
{
    return _value != Enum::GOTO_OFFSET;
}

bool AnimationBytecode::usesGotoLabel() const
{
    return _value == Enum::GOTO_ANIMATION;
}
