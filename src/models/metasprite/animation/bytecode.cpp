#include "bytecode.h"
#include <stdexcept>

using namespace UnTech::MetaSprite::Animation;

const std::map<Bytecode::Enum, std::string> Bytecode::enumMap = {
    { Bytecode::Enum::STOP, "STOP" },
    { Bytecode::Enum::GOTO_START, "GOTO_START" },
    { Bytecode::Enum::GOTO_ANIMATION, "GOTO_ANIMATION" },
    { Bytecode::Enum::GOTO_OFFSET, "GOTO_OFFSET" },
    { Bytecode::Enum::SET_FRAME_AND_WAIT_FRAMES, "SET_FRAME_AND_WAIT_FRAMES" },
    { Bytecode::Enum::SET_FRAME_AND_WAIT_TIME, "SET_FRAME_AND_WAIT_TIME" },
    { Bytecode::Enum::SET_FRAME_AND_WAIT_XVECL, "SET_FRAME_AND_WAIT_XVECL" },
    { Bytecode::Enum::SET_FRAME_AND_WAIT_YVECL, "SET_FRAME_AND_WAIT_YVECL" },
};

const std::map<std::string, Bytecode::Enum> Bytecode::stringMap = {
    { "STOP", Bytecode::Enum::STOP },
    { "GOTO_START", Bytecode::Enum::GOTO_START },
    { "GOTO_ANIMATION", Bytecode::Enum::GOTO_ANIMATION },
    { "GOTO_OFFSET", Bytecode::Enum::GOTO_OFFSET },
    { "SET_FRAME_AND_WAIT_FRAMES", Bytecode::Enum::SET_FRAME_AND_WAIT_FRAMES },
    { "SET_FRAME_AND_WAIT_TIME", Bytecode::Enum::SET_FRAME_AND_WAIT_TIME },
    { "SET_FRAME_AND_WAIT_XVECL", Bytecode::Enum::SET_FRAME_AND_WAIT_XVECL },
    { "SET_FRAME_AND_WAIT_YVECL", Bytecode::Enum::SET_FRAME_AND_WAIT_YVECL },
};

unsigned Bytecode::instructionSize() const
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

bool Bytecode::isValidTerminator() const
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

bool Bytecode::usesFrame() const
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

bool Bytecode::usesParameter() const
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

bool Bytecode::parameterUnsigned() const
{
    return _value != Enum::GOTO_OFFSET;
}

bool Bytecode::usesGotoLabel() const
{
    return _value == Enum::GOTO_ANIMATION;
}
