/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "bytecode.h"
#include "game-state.h"

namespace UnTech::Scripting {

constexpr unsigned N_FLAG_INSTRUCTIONS = GameState::MAX_FLAGS / 256;
constexpr unsigned MAX_OPCODES = 256 / 2;

const std::vector<Instruction> BytecodeInput::BASE_INSTRUCTIONS{
    { idstring{ "End_Script" }, {} },
    { idstring{ "Set_Flag" }, { ArgumentType::Flag } },
    { idstring{ "Clear_Flag" }, { ArgumentType::Flag } },
    { idstring{ "Set_Word" }, { ArgumentType::Word, ArgumentType::ImmediateU16 } },
    { idstring{ "Add_To_Word" }, { ArgumentType::Word, ArgumentType::ImmediateU16 } },
    { idstring{ "Subtract_From_Word" }, { ArgumentType::Word, ArgumentType::ImmediateU16 } },
    { idstring{ "Increment_Word" }, { ArgumentType::Word } },
    { idstring{ "Decrement_Word" }, { ArgumentType::Word } },
};

static unsigned argumentSize(const ArgumentType type)
{
    switch (type) {
    case ArgumentType::Unused:
        return 0;

    case ArgumentType::Flag:
        return 1;

    case ArgumentType::Word:
        return 1;

    case ArgumentType::ImmediateU16:
        return 2;
    }

    throw std::invalid_argument("Unknown ArgumentType");
}

std::shared_ptr<const BytecodeMapping> compileBytecode(const BytecodeInput& input, ErrorList& err)
{
    bool valid = true;
    auto addError = [&](const auto... msg) {
        err.addErrorString(msg...);
        valid = false;
    };
    auto addInstructionError = [&](const Instruction& inst, const auto... msg) {
        err.addErrorString("Instruction ", inst.name, ": ", msg...);
        valid = false;
    };

    auto out = std::make_shared<BytecodeMapping>();

    unsigned currentOpcode = 0;

    auto addInstructions = [&](const std::vector<Instruction>& instructions) {
        for (const auto& inst : instructions) {
            InstructionData iData;

            iData.opcode = currentOpcode;
            iData.arguments = inst.arguments;

            iData.instructionSize = 0;
            iData.nArguments = 0;
            for (auto arg : inst.arguments) {
                iData.instructionSize += argumentSize(arg);

                if (arg != ArgumentType::Unused) {
                    iData.nArguments++;
                }
            }

            const auto [it, created] = out->instructions.emplace(inst.name, iData);

            if (!inst.name.isValid()) {
                addInstructionError(inst, "Missing instruction name");
            }
            else if (!created) {
                addInstructionError(inst, "Duplicate name detected");
            }

            const unsigned nFlags = std::count(inst.arguments.begin(), inst.arguments.end(), ArgumentType::Flag);
            if (nFlags > 1) {
                addInstructionError(inst, "Too many flag arguments (only allowed 1)");
            }

            currentOpcode += nFlags == 0 ? 1 : N_FLAG_INSTRUCTIONS;
        }
    };
    addInstructions(BytecodeInput::BASE_INSTRUCTIONS);
    addInstructions(input.instructions);

    if (currentOpcode > MAX_OPCODES) {
        addError("Too many opcodes (", currentOpcode, ", max: ", MAX_OPCODES, ")");
    }

    if (!valid) {
        return nullptr;
    }

    // populate instructionNames
    {
        out->instructionNames.resize(input.BASE_INSTRUCTIONS.size() + input.instructions.size());

        auto it = std::transform(input.BASE_INSTRUCTIONS.begin(), input.BASE_INSTRUCTIONS.end(),
                                 out->instructionNames.begin(), [](auto& inst) { return inst.name; });

        it = std::transform(input.instructions.begin(), input.instructions.end(),
                            it, [](auto& inst) { return inst.name; });

        assert(it == out->instructionNames.end());

        std::sort(out->instructionNames.begin(), out->instructionNames.end());
    }

    assert(valid);

    return out;
}

static std::array<std::string, N_FLAG_INSTRUCTIONS> FLAG_ARGUMENT_SUFFIXES{
    "_Flag0",
    "_Flag1",
};

static std::array<std::string, 4> ARGUMENT_SUFFIXES{
    "",
    "_ ERROR _",
    "_Word",
    "_ImmU16",
};

// Assumes BytecodeInput is valid
void writeBytecodeFunctionTable(const BytecodeInput& input, std::ostream& out)
{
    out << "code()\n"
           "Project.BytecodeFunctionTable:\n";

    unsigned currentOpcode = 0;

    auto writeInstructions = [&](const std::vector<Instruction>& instructions) {
        for (const auto& inst : instructions) {
            const bool hasArgument = std::any_of(inst.arguments.begin(), inst.arguments.end(),
                                                 [](auto a) { return a != ArgumentType::Unused; });

            const bool hasFlagArgument = std::any_of(inst.arguments.begin(), inst.arguments.end(),
                                                     [](auto a) { return a == ArgumentType::Flag; });

            for (auto& flagName : FLAG_ARGUMENT_SUFFIXES) {
                out << "  dw Scripting.Bytecode." << inst.name;

                if (hasArgument) {
                    out << "__";
                }

                for (auto arg : inst.arguments) {
                    if (arg == ArgumentType::Flag) {
                        out << flagName;
                    }
                    else {
                        out << ARGUMENT_SUFFIXES.at(unsigned(arg));
                    }
                }
                out << '\n';

                currentOpcode++;

                if (hasFlagArgument == false) {
                    break;
                }
            }
        }
    };
    writeInstructions(BytecodeInput::BASE_INSTRUCTIONS);
    writeInstructions(input.instructions);

    for (unsigned i = currentOpcode; i < MAX_OPCODES; i++) {
        out << "  dw Scripting.Bytecode.InvalidOpcode\n";
    }

    out << "constant Project.BytecodeFunctionTable.size = pc() - Project.BytecodeFunctionTable"
           "\n\n";
}

}
