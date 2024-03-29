/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/errorlist.h"
#include "models/common/idstring.h"
#include <array>
#include <unordered_map>

namespace UnTech::Scripting {

enum class ArgumentType {
    Unused,

    Flag,
    Word,
    ImmediateU16,

    RoomScript,
    EntityGroup,
    Room,
    RoomEntrance,
};

struct Instruction {
    idstring name;

    std::array<ArgumentType, 2> arguments;

    bool yields = false;

    bool operator==(const Instruction&) const = default;
};

struct BytecodeInput {
    static const std::vector<Instruction> BASE_INSTRUCTIONS;

    std::vector<Instruction> instructions;

    bool operator==(const BytecodeInput&) const = default;
};

struct InstructionData {
    unsigned instructionSize;
    unsigned nArguments;

    uint8_t opcode;
    std::array<ArgumentType, 2> arguments;
    bool yields;
};

struct BytecodeMapping {
    constexpr static uint8_t endScriptOpcode = 0;
    constexpr static uint8_t gotoOpcode = 1;
    constexpr static uint8_t branchIfWordEqualOpcode = 2;
    constexpr static uint8_t branchIfWordNotEqualOpcode = 3;
    constexpr static uint8_t branchIfWordLessThanOpcode = 4;
    constexpr static uint8_t branchIfWordGreaterThanEqualOpcode = 5;

    constexpr static uint8_t branchIfFlagSetOpcode = 6;
    uint8_t branchIfFlagClearOpcode;

    std::unordered_map<idstring, InstructionData> instructions;

    std::vector<idstring> instructionNames;
};

std::shared_ptr<const BytecodeMapping>
compileBytecode(const BytecodeInput& input, ErrorList& err);

void writeBytecodeFunctionTable(const BytecodeInput& input, StringStream& out);

}
