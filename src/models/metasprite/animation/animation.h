#pragma once

#include "bytecode.h"
#include "../common.h"
#include "models/common/capped_vector.h"
#include <map>
#include <string>
#include <vector>

namespace UnTech {
namespace MetaSprite {
namespace Animation {

struct Instruction {
    typedef capped_vector<Instruction, MAX_ANIMATION_INSTRUCTIONS> list_t;

    Bytecode operation;
    std::string gotoLabel;
    NameReference frame;
    int parameter;

    Instruction() = default;
    Instruction(const Instruction&) = default;
    Instruction(Instruction&&) = default;

    // ::TODO isValid - given frame name list::
};

struct Animation {
    // ::TODO replace with idstring::
    typedef std::map<std::string, Animation> map_t;

    Instruction::list_t instructions;

    Animation() = default;
    Animation(const Animation&) = default;
    Animation(Animation&&) = default;

    // ::TODO isValid - given frame name list::
};
}
}
}
