/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "bytecode.h"
#include "game-state.h"
#include "scripting-error.h"
#include "models/common/iterators.h"
#include <climits>

namespace UnTech::Scripting {

constexpr unsigned N_SPECIAL_RESUME_OPCODES = 6;
constexpr unsigned N_FLAG_INSTRUCTIONS = GameState::MAX_FLAGS / 256;
constexpr unsigned MAX_OPCODES = 256 / 2;

template <typename... Args>
std::unique_ptr<BytecodeError> instructionError(const Instruction& inst, const unsigned index, const Args... msg)
{
    return std::make_unique<BytecodeError>(BytecodeErrorType::INSTRUCTION, index,
                                           stringBuilder("Instruction ", inst.name, ": ", msg...));
}

const std::vector<Instruction> BytecodeInput::BASE_INSTRUCTIONS{
    { idstring{ "End_Script" }, {}, false },
    { idstring{ "Set_Flag" }, { ArgumentType::Flag }, false },
    { idstring{ "Clear_Flag" }, { ArgumentType::Flag }, false },
    { idstring{ "Set_Word" }, { ArgumentType::Word, ArgumentType::ImmediateU16 }, false },
    { idstring{ "Add_To_Word" }, { ArgumentType::Word, ArgumentType::ImmediateU16 }, false },
    { idstring{ "Subtract_From_Word" }, { ArgumentType::Word, ArgumentType::ImmediateU16 }, false },
    { idstring{ "Increment_Word" }, { ArgumentType::Word }, false },
    { idstring{ "Decrement_Word" }, { ArgumentType::Word }, false },
    { idstring{ "Sleep_AnimationTicks" }, { ArgumentType::ImmediateU16 }, true },
    { idstring{ "Start_Script" }, { ArgumentType::RoomScript }, false },
    { idstring{ "Spawn_Entity_Group" }, { ArgumentType::EntityGroup }, false },
    { idstring{ "Load_Room" }, { ArgumentType::Room, ArgumentType::RoomEntrance }, false },
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

    case ArgumentType::RoomScript:
        return 1;

    case ArgumentType::EntityGroup:
        return 1;

    case ArgumentType::Room:
        return 1;

    case ArgumentType::RoomEntrance:
        return 1;
    }

    throw std::invalid_argument("Unknown ArgumentType");
}

static unsigned numberOfOpcodes(const Instruction& inst)
{
    const unsigned nFlags = std::count(inst.arguments.begin(), inst.arguments.end(), ArgumentType::Flag);
    return nFlags == 0 ? 1 : N_FLAG_INSTRUCTIONS;
}

std::shared_ptr<const BytecodeMapping> compileBytecode(const BytecodeInput& input, ErrorList& err)
{
    bool valid = true;
    auto addError = [&](const auto... msg) {
        err.addErrorString(msg...);
        valid = false;
    };
    auto addInstructionError = [&](const Instruction& inst, unsigned index, const auto... msg) {
        err.addError(instructionError(inst, index, msg...));
        valid = false;
    };

    auto out = std::make_shared<BytecodeMapping>();

    unsigned currentOpcode = 0;
    auto addInstruction = [&](const Instruction& inst, const unsigned index) {
        InstructionData iData;

        iData.opcode = currentOpcode;
        iData.arguments = inst.arguments;
        iData.yields = inst.yields;

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
            addInstructionError(inst, index, "Missing instruction name");
        }
        else if (!created) {
            addInstructionError(inst, index, "Duplicate instruction name detected");
        }

        const unsigned nFlags = std::count(inst.arguments.begin(), inst.arguments.end(), ArgumentType::Flag);
        if (nFlags > 1) {
            addInstructionError(inst, index, "Too many flag arguments (only allowed 1)");
        }

        if (nFlags > 0 && inst.yields) {
            // This simplifies the `project.inc` generation
            addInstructionError(inst, index, "A yielding instruction cannot have a flag argument");
        }

        currentOpcode += nFlags == 0 ? 1 : N_FLAG_INSTRUCTIONS;
    };

    // The first bytecode instruction is hard coded to use opcode 0
    assert(input.BASE_INSTRUCTIONS.at(0).name.str() == "End_Script");
    assert(out->endScriptOpcode == currentOpcode);
    addInstruction(input.BASE_INSTRUCTIONS.at(0), INT_MAX);

    // Setup branch/conditional opcodes
    out->branchIfFlagClearOpcode = out->branchIfFlagSetOpcode + N_FLAG_INSTRUCTIONS;
    currentOpcode = out->branchIfFlagClearOpcode + N_FLAG_INSTRUCTIONS;

    for (const auto& inst : skip_first_element(input.BASE_INSTRUCTIONS)) {
        addInstruction(inst, INT_MAX);
    }

    // Add custom instructions
    for (auto [index, inst] : const_enumerate(input.instructions)) {
        addInstruction(inst, index);
    }

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

static std::array<std::string, 8> ARGUMENT_SUFFIXES{
    "",
    "_ ERROR _", // Flag
    "_Word",
    "_ImmU16",
    "_RoomScript",
    "_EntityGroup",
    "_Room",
    "_RoomEntrance",
};

// Assumes BytecodeInput is valid
void writeBytecodeFunctionTable(const BytecodeInput& input, std::ostream& out)
{
    out << "code()\n"
           "Project.BytecodeFunctionTable:\n";

    unsigned currentOpcode = 0;

    auto processInstructions = [&](auto f) {
        for (const auto& inst : skip_first_element(input.BASE_INSTRUCTIONS)) {
            f(inst);
        }
        for (auto& inst : input.instructions) {
            f(inst);
        }
    };

    auto writeHardCodedOpcode = [&](const std::string& name, const uint8_t opcode) {
        assert(currentOpcode == opcode);
        out << "  dw Scripting.Bytecode." << name << '\n';
        currentOpcode++;
    };

    auto writeFlagBranchOpcode = [&](const std::string& name) {
        for (auto& arg : FLAG_ARGUMENT_SUFFIXES) {
            out << "  dw Scripting.Bytecode." << name << arg << "_PC\n";
            currentOpcode++;
        }
    };

    // Write branch/conditional opcodes
    writeHardCodedOpcode("End_Script", BytecodeMapping::endScriptOpcode);
    writeHardCodedOpcode("Goto___PC", BytecodeMapping::gotoOpcode);
    writeHardCodedOpcode("BranchIfWordEqual___Word_ImmU16_PC", BytecodeMapping::branchIfWordEqualOpcode);
    writeHardCodedOpcode("BranchIfWordNotEqual___Word_ImmU16_PC", BytecodeMapping::branchIfWordNotEqualOpcode);
    writeHardCodedOpcode("BranchIfWordLessThan___Word_ImmU16_PC", BytecodeMapping::branchIfWordLessThanOpcode);
    writeHardCodedOpcode("BranchIfWordGreaterThanEqual___Word_ImmU16_PC", BytecodeMapping::branchIfWordGreaterThanEqualOpcode);

    assert(currentOpcode == BytecodeMapping::branchIfFlagSetOpcode);
    writeFlagBranchOpcode("BranchIfFlagSet__");
    writeFlagBranchOpcode("BranchIfFlagClear__");
    assert(currentOpcode == BytecodeMapping::branchIfFlagSetOpcode + 2 * N_FLAG_INSTRUCTIONS);

    const unsigned startOfStatementInstructions = currentOpcode;

    processInstructions([&](const Instruction& inst) {
        const bool hasArgument = std::any_of(inst.arguments.begin(), inst.arguments.end(),
                                             [](auto a) { return a != ArgumentType::Unused; });

        const bool hasFlagArgument = std::any_of(inst.arguments.begin(), inst.arguments.end(),
                                                 [](auto a) { return a == ArgumentType::Flag; });

        for (auto& flagName : FLAG_ARGUMENT_SUFFIXES) {
            out << "  dw Scripting.Bytecode." << inst.name;

            if (hasArgument) {
                out << "__";

                for (auto arg : inst.arguments) {
                    if (arg == ArgumentType::Flag) {
                        out << flagName;
                    }
                    else {
                        out << ARGUMENT_SUFFIXES.at(unsigned(arg));
                    }
                }
            }
            out << '\n';

            currentOpcode++;

            if (hasFlagArgument == false) {
                break;
            }
        }
    });

    for ([[maybe_unused]] const auto i : range(currentOpcode, MAX_OPCODES)) {
        out << "  dw Scripting.Bytecode.InvalidOpcode\n";
    }

    out << "constant Project.BytecodeFunctionTable.size = pc() - Project.BytecodeFunctionTable"
           "\n"
           "\n"
           "code()\n"
           "Project.BytecodeResumeFunctionTable:\n";

    const char* const blankResumeLine = "  dw Scripting.Bytecode.End_Script___Resume\n";

    for (const auto i : range(N_SPECIAL_RESUME_OPCODES)) {
        out << "  dw Scripting.Bytecode._Special___Resume_" << i * 2 << '\n';
    }

    assert(N_SPECIAL_RESUME_OPCODES < startOfStatementInstructions);
    for (currentOpcode = N_SPECIAL_RESUME_OPCODES; currentOpcode < startOfStatementInstructions; currentOpcode++) {
        out << blankResumeLine;
    }

    processInstructions([&](const Instruction& inst) {
        if (inst.yields) {
            const bool hasArgument = std::any_of(inst.arguments.begin(), inst.arguments.end(),
                                                 [](auto a) { return a != ArgumentType::Unused; });

            out << "  dw Scripting.Bytecode." << inst.name;

            if (hasArgument) {
                out << "__";

                for (auto arg : inst.arguments) {
                    out << ARGUMENT_SUFFIXES.at(unsigned(arg));
                }
            }

            out << "___Resume\n";
        }
        else {
            const unsigned nOpcodes = numberOfOpcodes(inst);
            for ([[maybe_unused]] const auto i : range(nOpcodes)) {
                out << blankResumeLine;
            }
            currentOpcode += nOpcodes;
        }
    });

    for ([[maybe_unused]] const auto i : range(currentOpcode, MAX_OPCODES)) {
        out << blankResumeLine;
    }

    out << "constant Project.BytecodeResumeFunctionTable.size = pc() - Project.BytecodeResumeFunctionTable"
           "\n"
           "\n"
           "// indexes into Project.BytecodeResumeFunctionTable\n"
           "namespace Project.BytecodeOpcodes.Yielding {\n"
           "  constant N_SPECIAL_RESUME_OPCODES = "
        << N_SPECIAL_RESUME_OPCODES << '\n';

    currentOpcode = startOfStatementInstructions;

    processInstructions([&](const Instruction& inst) {
        if (inst.yields) {
            out << "  constant " << inst.name << " = " << currentOpcode * 2 << '\n';
        }
        const unsigned nOpcodes = numberOfOpcodes(inst);
        currentOpcode += nOpcodes;
    });

    out << "}\n\n";
}

}
