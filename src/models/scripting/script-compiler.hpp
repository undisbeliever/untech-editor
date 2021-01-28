/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "bytecode.h"
#include "game-state.h"
#include "script.h"
#include "models/common/string.h"

namespace UnTech::Scripting {

class ScriptCompiler {

private:
    const BytecodeMapping& bytecode;
    const GameStateData& gameState;
    const idstring roomId;

    std::vector<uint8_t>& data;

    idstring scriptName;
    unsigned depth;

    ErrorList& err;
    bool valid;

public:
    ScriptCompiler(std::vector<uint8_t>& data, const idstring& roomId,
                   const BytecodeMapping& bytecode, const GameStateData& gameState,
                   ErrorList& e)
        : bytecode(bytecode)
        , gameState(gameState)
        , roomId(roomId)
        , data(data)
        , depth(0)
        , err(e)
        , valid(true)
    {
        // ::TODO load room variables::
    }

    bool isValid() const { return valid; }

    unsigned compileScript(const Script& script)
    {
        const unsigned scriptPos = data.size();

        scriptName = script.name;

        // ::TODO load script variables::

        compileStatements(script.statements);

        data.push_back(bytecode.endScriptOpcode);

        assert(depth == 0);

        return scriptPos;
    }

private:
    template <typename... T>
    void addError(T... msg)
    {
        // ::TODO include line number in the error::

        err.addErrorString("Error compiling script `", scriptName, "`: ", msg...);
        valid = false;
    }

    template <typename... T>
    void addWarning(T... msg)
    {
        // ::TODO include line number in the error::

        err.addWarningString("Script `", scriptName, "`: ", msg...);
        valid = false;
    }

    void compileStatements(const std::vector<ScriptNode>& statements)
    {
        depth++;
        if (depth > Script::MAX_DEPTH) {
            addError("Script too deep");
            return;
        }

        for (auto& s : statements) {
            std::visit(*this, s);
        }

        depth--;
    }

    unsigned getFlagId(const std::string& name)
    {
        auto it = gameState.flags.find(name);
        if (it != gameState.flags.end()) {
            if (!it->second.allowedInRoom(roomId)) {
                addError("Invalid flag `", name, "` (it is private and belongs to room `", roomId, "`)");
            }
            return it->second.index;
        }

        addError("Unknown flag: ", name);
        return 0;
    }

    unsigned getWordId(const std::string& name)
    {
        auto it = gameState.words.find(name);
        if (it != gameState.words.end()) {
            if (!it->second.allowedInRoom(roomId)) {
                addError("Invalid word `", name, "` (it is private and belongs to room `", roomId, "`)");
            }
            return it->second.index;
        }

        addError("Unknown word: ", name);
        return 0;
    }

    unsigned getImmediateU16(const std::string& value)
    {
        auto u16 = String::toUint16(value);

        if (u16) {
            return *u16;
        }
        else {
            addError("Invalid argument: ", value);
            return 0;
        }
    }

    void statementArgument(const ArgumentType& type, const std::string& value, const size_t bytecodePos)
    {
        switch (type) {
        case ArgumentType::Unused: {
        } break;

        case ArgumentType::Flag: {
            const unsigned flagId = getFlagId(value);

            // This is safe as only one Flag argument is allowed per bytecode instruction.
            data.at(bytecodePos) += flagId / 0x100;
            data.push_back(flagId & 0xff);
        } break;

        case ArgumentType::Word: {
            data.push_back(getWordId(value));
        } break;

        case ArgumentType::ImmediateU16: {
            uint16_t i = getImmediateU16(value);

            data.push_back(i & 0xff);
            data.push_back(i >> 8);
        } break;
        }
    }

    // Returns the position of the branch argument in the conditional statement
    unsigned conditional(const Conditional& conditional)
    {
        switch (conditional.type) {
        case ConditionalType::Word: {
            uint8_t opcode = bytecode.endScriptOpcode;
            const uint8_t wordId = getWordId(conditional.variable);
            const uint16_t value = getImmediateU16(conditional.value);

            switch (conditional.comparison) {
            case ComparisonType::Equal:
                opcode = bytecode.branchIfWordNotEqualOpcode;
                break;

            case ComparisonType::NotEqual:
                opcode = bytecode.branchIfWordEqualOpcode;
                break;

            case ComparisonType::LessThan:
                opcode = bytecode.branchIfWordGreaterThanEqualOpcode;
                break;

            case ComparisonType::GreaterThanEqual:
                opcode = bytecode.branchIfWordLessThanOpcode;
                break;

            case ComparisonType::Set:
            case ComparisonType::Clear: {
                addError("Invalid comparison type for word variable");
                break;
            }
            }

            data.push_back(opcode);
            data.push_back(wordId);
            data.push_back(value & 0xff);
            data.push_back(value >> 8);
            data.push_back(0);
            data.push_back(0);
            return data.size() - 2;
        }

        case ConditionalType::Flag: {
            uint8_t opcode = bytecode.endScriptOpcode;
            const unsigned flagId = getFlagId(conditional.variable);

            switch (conditional.comparison) {
            case ComparisonType::Set:
                opcode = bytecode.branchIfFlagClearOpcode + (flagId / 0x100);
                break;

            case ComparisonType::Clear:
                opcode = bytecode.branchIfFlagSetOpcode + (flagId / 0x100);
                break;

            case ComparisonType::Equal:
            case ComparisonType::NotEqual:
            case ComparisonType::LessThan:
            case ComparisonType::GreaterThanEqual:
                addError("Invalid comparison type for flag variable");
                break;
            }

            data.push_back(opcode);
            data.push_back(flagId & 0xff);
            data.push_back(0);
            data.push_back(0);
            return data.size() - 2;
        }
        }

        throw std::invalid_argument("Invalid ConditionalType");
    }

    void writeWordAt(unsigned i, uint16_t word)
    {
        data.at(i) = word & 0xff;
        data.at(i + 1) = word >> 8;
    }

public:
    void operator()(const Statement& statement)
    {
        const auto bcIt = bytecode.instructions.find(statement.opcode);
        if (bcIt == bytecode.instructions.end()) {
            addError("Unknown instruction: ", statement.opcode);
            return;
        }
        const auto& bc = bcIt->second;

        const auto bytecodePos = data.size();

        data.push_back(bc.opcode);

        assert(bc.arguments.size() == statement.arguments.size());
        for (unsigned i = 0; i < bc.arguments.size(); i++) {
            statementArgument(bc.arguments.at(i), statement.arguments.at(i), bytecodePos);
        }
    }

    void operator()(const IfStatement& ifStatement)
    {
        const unsigned branchPos = conditional(ifStatement.condition);

        if (ifStatement.thenStatements.empty()) {
            addWarning("Empty body in an if statement");
        }

        compileStatements(ifStatement.thenStatements);

        if (ifStatement.elseStatements.empty()) {
            // write PC to conditional branch instruction
            writeWordAt(branchPos, data.size());
        }
        else {
            const unsigned endIfGotoPos = data.size() + 1;
            data.push_back(bytecode.gotoOpcode);
            data.push_back(0);
            data.push_back(0);

            writeWordAt(branchPos, data.size());

            compileStatements(ifStatement.elseStatements);

            writeWordAt(endIfGotoPos, data.size());
        }
    }

    void operator()(const Comment&)
    {
        // Ignore comments
    }
};
}
