/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/errorlist.h"
#include "models/common/idstring.h"

namespace UnTech::Scripting {

enum class ArgumentType {
    Unused,

    Flag,
    Word,
    ImmediateU16,
};

struct Instruction {
    idstring name;

    std::array<ArgumentType, 2> arguments;

    bool operator==(const Instruction& o) const
    {
        return name == o.name
               && arguments == o.arguments;
    }
};

struct BytecodeInput {
    static const std::vector<Instruction> BASE_INSTRUCTIONS;

    constexpr static uint8_t END_SCRIPT_OPCODE = 0;

    std::vector<Instruction> instructions;

    bool operator==(const BytecodeInput& o) const
    {
        return instructions == o.instructions;
    }
};

struct InstructionData {
    unsigned instructionSize;
    unsigned nArguments;

    uint8_t opcode;
    std::array<ArgumentType, 2> arguments;
};

struct BytecodeMapping {
    std::unordered_map<idstring, InstructionData> instructions;

    std::vector<idstring> instructionNames;
};

std::shared_ptr<const BytecodeMapping>
compileBytecode(const BytecodeInput& input, ErrorList& err);

void writeBytecodeFunctionTable(const BytecodeInput& input, std::ostream& out);

}
