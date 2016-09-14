#pragma once

#include "bytecode.h"
#include "../common.h"
#include <map>
#include <string>
#include <vector>

namespace UnTech {
namespace MetaSprite {
namespace Animation {

struct Instruction {
    Bytecode operation;
    std::string gotoLabel;
    FrameReference frame;
    int parameter;

    Instruction() = default;
    Instruction(const Instruction&) = default;
    Instruction(Instruction&&) = default;

    // ::TODO isValid - given frame name list::
};

struct Animation {
    // ::TODO replace with idstring::
    typedef std::map<std::string, Animation> map_t;

    std::vector<Instruction> instructions;

    Animation() = default;
    Animation(const Animation&) = default;
    Animation(Animation&&) = default;

    // ::TODO isValid - given frame name list::
};
}
}
}
