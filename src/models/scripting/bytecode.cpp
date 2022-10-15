/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "bytecode.h"
#include "game-state.h"
#include "scripting-error.h"
#include "models/common/exceptions.h"
#include "models/common/iterators.h"
#include "models/common/stringstream.h"
#include <climits>

namespace UnTech::Scripting {

constexpr unsigned N_SPECIAL_RESUME_OPCODES = 6;
constexpr unsigned N_FLAG_INSTRUCTIONS = GameState::MAX_FLAGS / 256;
constexpr unsigned MAX_OPCODES = 256 / 2;

template <typename... Args>
std::unique_ptr<BytecodeError> instructionError(const Instruction& inst, const unsigned index, const Args... msg)
{
    return std::make_unique<BytecodeError>(BytecodeErrorType::INSTRUCTION, index,
                                           stringBuilder(u8"Instruction ", inst.name, u8": ", msg...));
}

const std::vector<Instruction> BytecodeInput::BASE_INSTRUCTIONS{
    { u8"End_Script"_id, {}, false },
    { u8"Set_Flag"_id, { ArgumentType::Flag }, false },
    { u8"Clear_Flag"_id, { ArgumentType::Flag }, false },
    { u8"Set_Word"_id, { ArgumentType::Word, ArgumentType::ImmediateU16 }, false },
    { u8"Add_To_Word"_id, { ArgumentType::Word, ArgumentType::ImmediateU16 }, false },
    { u8"Subtract_From_Word"_id, { ArgumentType::Word, ArgumentType::ImmediateU16 }, false },
    { u8"Increment_Word"_id, { ArgumentType::Word }, false },
    { u8"Decrement_Word"_id, { ArgumentType::Word }, false },
    { u8"Sleep_AnimationTicks"_id, { ArgumentType::ImmediateU16 }, true },
    { u8"Start_Script"_id, { ArgumentType::RoomScript }, false },
    { u8"Spawn_Entity_Group"_id, { ArgumentType::EntityGroup }, false },
    { u8"Load_Room"_id, { ArgumentType::Room, ArgumentType::RoomEntrance }, false },
};

static unsigned argumentSize(const ArgumentType type)
{
    switch (type) {
    case ArgumentType::Unused:
        return 0;

    case ArgumentType::Flag:
    case ArgumentType::Word:
        return 1;

    case ArgumentType::ImmediateU16:
        return 2;

    case ArgumentType::RoomScript:
    case ArgumentType::EntityGroup:
    case ArgumentType::Room:
    case ArgumentType::RoomEntrance:
        return 1;
    }

    throw invalid_argument(u8"Unknown ArgumentType");
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
        InstructionData iData{};

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
            addInstructionError(inst, index, u8"Missing instruction name");
        }
        else if (!created) {
            addInstructionError(inst, index, u8"Duplicate instruction name detected");
        }

        const unsigned nFlags = std::count(inst.arguments.begin(), inst.arguments.end(), ArgumentType::Flag);
        if (nFlags > 1) {
            addInstructionError(inst, index, u8"Too many flag arguments (only allowed 1)");
        }

        if (nFlags > 0 && inst.yields) {
            // This simplifies the `project.inc` generation
            addInstructionError(inst, index, u8"A yielding instruction cannot have a flag argument");
        }

        currentOpcode += nFlags == 0 ? 1 : N_FLAG_INSTRUCTIONS;
    };

    // The first bytecode instruction is hard coded to use opcode 0
    assert(input.BASE_INSTRUCTIONS.at(0).name.str() == u8"End_Script");
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
        addError(u8"Too many opcodes (", currentOpcode, u8", max: ", MAX_OPCODES, u8")");
    }

    if (!valid) {
        return nullptr;
    }

    // populate instructionNames
    {
        out->instructionNames.resize(input.BASE_INSTRUCTIONS.size() + input.instructions.size());

        auto it = std::transform(input.BASE_INSTRUCTIONS.begin(), input.BASE_INSTRUCTIONS.end(),
                                 out->instructionNames.begin(), [](const auto& inst) { return inst.name; });

        it = std::transform(input.instructions.begin(), input.instructions.end(),
                            it, [](const auto& inst) { return inst.name; });

        assert(it == out->instructionNames.end());

        std::sort(out->instructionNames.begin(), out->instructionNames.end());
    }

    assert(valid);

    return out;
}

static constexpr std::array<std::u8string_view, N_FLAG_INSTRUCTIONS> FLAG_ARGUMENT_SUFFIXES{
    u8"_Flag0",
    u8"_Flag1",
};

static constexpr std::array<std::u8string_view, 8> ARGUMENT_SUFFIXES{
    u8"",
    u8"_ ERROR _", // Flag
    u8"_Word",
    u8"_ImmU16",
    u8"_RoomScript",
    u8"_EntityGroup",
    u8"_Room",
    u8"_RoomEntrance",
};

// Assumes BytecodeInput is valid
void writeBytecodeFunctionTable(const BytecodeInput& input, StringStream& out)
{
    out.write(u8"code()\n"
              u8"Project.BytecodeFunctionTable:\n");

    unsigned currentOpcode = 0;

    auto processInstructions = [&](auto f) {
        for (const auto& inst : skip_first_element(input.BASE_INSTRUCTIONS)) {
            f(inst);
        }
        for (auto& inst : input.instructions) {
            f(inst);
        }
    };

    auto writeHardCodedOpcode = [&](const std::u8string& name, const uint8_t opcode) {
        assert(currentOpcode == opcode);
        out.write(u8"  dw Scripting.Bytecode.", name, u8"\n");
        currentOpcode++;
    };

    auto writeFlagBranchOpcode = [&](const std::u8string& name) {
        for (auto& arg : FLAG_ARGUMENT_SUFFIXES) {
            out.write(u8"  dw Scripting.Bytecode.", name, arg, u8"_PC\n");
            currentOpcode++;
        }
    };

    // Write branch/conditional opcodes
    writeHardCodedOpcode(u8"End_Script", BytecodeMapping::endScriptOpcode);
    writeHardCodedOpcode(u8"Goto___PC", BytecodeMapping::gotoOpcode);
    writeHardCodedOpcode(u8"BranchIfWordEqual___Word_ImmU16_PC", BytecodeMapping::branchIfWordEqualOpcode);
    writeHardCodedOpcode(u8"BranchIfWordNotEqual___Word_ImmU16_PC", BytecodeMapping::branchIfWordNotEqualOpcode);
    writeHardCodedOpcode(u8"BranchIfWordLessThan___Word_ImmU16_PC", BytecodeMapping::branchIfWordLessThanOpcode);
    writeHardCodedOpcode(u8"BranchIfWordGreaterThanEqual___Word_ImmU16_PC", BytecodeMapping::branchIfWordGreaterThanEqualOpcode);

    assert(currentOpcode == BytecodeMapping::branchIfFlagSetOpcode);
    writeFlagBranchOpcode(u8"BranchIfFlagSet__");
    writeFlagBranchOpcode(u8"BranchIfFlagClear__");
    assert(currentOpcode == BytecodeMapping::branchIfFlagSetOpcode + 2 * N_FLAG_INSTRUCTIONS);

    const unsigned startOfStatementInstructions = currentOpcode;

    processInstructions([&](const Instruction& inst) {
        const bool hasArgument = std::any_of(inst.arguments.begin(), inst.arguments.end(),
                                             [](auto a) { return a != ArgumentType::Unused; });

        const bool hasFlagArgument = std::any_of(inst.arguments.begin(), inst.arguments.end(),
                                                 [](auto a) { return a == ArgumentType::Flag; });

        for (auto& flagName : FLAG_ARGUMENT_SUFFIXES) {
            out.write(u8"  dw Scripting.Bytecode.", inst.name);

            if (hasArgument) {
                out.write(u8"__");

                for (auto arg : inst.arguments) {
                    if (arg == ArgumentType::Flag) {
                        out.write(flagName);
                    }
                    else {
                        out.write(ARGUMENT_SUFFIXES.at(unsigned(arg)));
                    }
                }
            }
            out.write(u8"\n");

            currentOpcode++;

            if (hasFlagArgument == false) {
                break;
            }
        }
    });

    for ([[maybe_unused]] const auto i : range(currentOpcode, MAX_OPCODES)) {
        out.write(u8"  dw Scripting.Bytecode.InvalidOpcode\n");
    }

    out.write(u8"constant Project.BytecodeFunctionTable.size = pc() - Project.BytecodeFunctionTable"
              u8"\n"
              u8"\n"
              u8"code()\n"
              u8"Project.BytecodeResumeFunctionTable:\n");

    const char8_t* const blankResumeLine = u8"  dw Scripting.Bytecode.End_Script___Resume\n";

    for (const auto i : range(N_SPECIAL_RESUME_OPCODES)) {
        out.write(u8"  dw Scripting.Bytecode._Special___Resume_", i * 2, u8"\n");
    }

    assert(N_SPECIAL_RESUME_OPCODES < startOfStatementInstructions);
    for (currentOpcode = N_SPECIAL_RESUME_OPCODES; currentOpcode < startOfStatementInstructions; currentOpcode++) {
        out.write(blankResumeLine);
    }

    processInstructions([&](const Instruction& inst) {
        if (inst.yields) {
            const bool hasArgument = std::any_of(inst.arguments.begin(), inst.arguments.end(),
                                                 [](auto a) { return a != ArgumentType::Unused; });

            out.write(u8"  dw Scripting.Bytecode.", inst.name);

            if (hasArgument) {
                out.write(u8"__");

                for (auto arg : inst.arguments) {
                    out.write(ARGUMENT_SUFFIXES.at(unsigned(arg)));
                }
            }

            out.write(u8"___Resume\n");
        }
        else {
            const unsigned nOpcodes = numberOfOpcodes(inst);
            for ([[maybe_unused]] const auto i : range(nOpcodes)) {
                out.write(blankResumeLine);
            }
            currentOpcode += nOpcodes;
        }
    });

    for ([[maybe_unused]] const auto i : range(currentOpcode, MAX_OPCODES)) {
        out.write(blankResumeLine);
    }

    out.write(u8"constant Project.BytecodeResumeFunctionTable.size = pc() - Project.BytecodeResumeFunctionTable"
              u8"\n"
              u8"\n"
              u8"// indexes into Project.BytecodeResumeFunctionTable\n"
              u8"namespace Project.BytecodeOpcodes.Yielding {\n"
              u8"  constant N_SPECIAL_RESUME_OPCODES = ",
              N_SPECIAL_RESUME_OPCODES, u8"\n");

    currentOpcode = startOfStatementInstructions;

    processInstructions([&](const Instruction& inst) {
        if (inst.yields) {
            out.write(u8"  constant ", inst.name, u8" = ", currentOpcode * 2, u8"\n");
        }
        const unsigned nOpcodes = numberOfOpcodes(inst);
        currentOpcode += nOpcodes;
    });

    out.write(u8"}\n\n");
}

}
