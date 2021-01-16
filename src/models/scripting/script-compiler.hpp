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

        data.push_back(BytecodeInput::END_SCRIPT_OPCODE);

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
};

}
