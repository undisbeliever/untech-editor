/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "bytecode.h"
#include "game-state.h"
#include "script.h"
#include "models/common/externalfilelist.h"
#include "models/common/iterators.h"
#include "models/common/string.h"
#include "models/rooms/rooms.h"

namespace UnTech::Scripting {

using GameStateDataValueMap = std::unordered_map<std::string, const GameStateData::Value>;

static inline std::unordered_map<std::string, unsigned>
setupTempVariables(const std::vector<idstring> tempVars, const char* typeName,
                   const GameStateDataValueMap& gameStateVars, const unsigned nGameStateVars, const unsigned maxVars,
                   ErrorList& err)
{
    auto addError = [&](const auto... msg) {
        err.addErrorString("Invalid temporary script ", typeName, ": ", msg...);
    };

    std::unordered_map<std::string, unsigned> map;
    map.reserve(tempVars.size());

    long varId = maxVars;

    for (auto& varName : tempVars) {
        varId--;

        if (!varName.isValid()) {
            addError("Missing name");
        }
        else if (gameStateVars.find(varName) != gameStateVars.end()) {
            addError("Name exists in GameState: ", varName);
        }
        else {
            const auto [it, inserted] = map.emplace(varName, varId);
            if (!inserted) {
                err.addErrorString("Duplicate temporary script ", typeName, " detected: ", varName);
            }
        }
    }

    if (tempVars.size() + nGameStateVars > maxVars) {
        addError("Too many temporary script ", typeName, " variables");
        map.clear();
    }
    else {
        assert(varId > 0 && unsigned(varId) >= nGameStateVars);
    }

    return map;
}

class ScriptCompiler {

private:
    const BytecodeMapping& bytecode;
    const GameStateData& gameState;
    const Rooms::RoomInput& room;
    const ExternalFileList<Rooms::RoomInput>& roomsList;

    // temporary room script variables
    const std::unordered_map<std::string, unsigned> tempFlags;
    const std::unordered_map<std::string, unsigned> tempWords;

    std::vector<uint8_t>& data;

    idstring scriptName;
    unsigned depth;

    ErrorList& err;
    bool valid;

public:
    ScriptCompiler(std::vector<uint8_t>& data, const Rooms::RoomInput& room,
                   const ExternalFileList<Rooms::RoomInput>& roomsList,
                   const BytecodeMapping& bytecode, const GameStateData& gameState,
                   ErrorList& e)
        : bytecode(bytecode)
        , gameState(gameState)
        , room(room)
        , roomsList(roomsList)
        , tempFlags(setupTempVariables(room.roomScripts.tempFlags, "flag", gameState.flags, gameState.nFlags, GameState::MAX_FLAGS, e))
        , tempWords(setupTempVariables(room.roomScripts.tempWords, "word", gameState.words, gameState.nWords, GameState::MAX_WORDS, e))
        , data(data)
        , depth(0)
        , err(e)
        , valid(true)
    {
        valid &= tempFlags.size() == room.roomScripts.tempFlags.size();
        valid &= tempWords.size() == room.roomScripts.tempWords.size();
    }

    bool isValid() const { return valid; }

    unsigned compileScript(const Script& script)
    {
        const unsigned scriptPos = data.size();

        scriptName = script.name;

        if (script.name == STARTUP_SCRIPT_NAME) {
            if (&script != &room.roomScripts.startupScript) {
                addError("Invalid script name");
            }
        }

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
        auto localIt = tempFlags.find(name);
        if (localIt != tempFlags.end()) {
            return localIt->second;
        }

        auto it = gameState.flags.find(name);
        if (it != gameState.flags.end()) {
            if (!it->second.allowedInRoom(room.name)) {
                addError("Invalid flag `", name, "` (it is private and belongs to room `", room.name, "`)");
            }
            return it->second.index;
        }

        addError("Unknown flag: ", name);
        return 0;
    }

    unsigned getWordId(const std::string& name)
    {
        auto localIt = tempWords.find(name);
        if (localIt != tempWords.end()) {
            return localIt->second;
        }

        auto it = gameState.words.find(name);
        if (it != gameState.words.end()) {
            if (!it->second.allowedInRoom(room.name)) {
                addError("Invalid word `", name, "` (it is private and belongs to room `", room.name, "`)");
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

    unsigned getRoomScriptId(const std::string& name)
    {
        const auto s = room.roomScripts.scripts.indexOf(name);

        if (s > room.roomScripts.scripts.size()) {
            addError("Unknown script: ", name);
            return 0;
        }

        return s;
    }

    unsigned getEntityGroupId(const std::string& name)
    {
        const auto s = room.entityGroups.indexOf(name);

        if (s > room.entityGroups.size()) {
            addError("Unknown entity group: ", name);
            return 0;
        }

        return s;
    }

    unsigned getRoomId(const std::string& name)
    {
        const auto s = roomsList.indexOf(name);

        if (s > roomsList.size()) {
            addError("Unknown room: ", name);
            return 0;
        }

        return s;
    }

    unsigned getRoomEntranceId(const std::string& name, const Rooms::RoomInput& r)
    {
        const auto s = r.entrances.indexOf(name);

        if (s > r.entrances.size()) {
            addError("Unknown room entrance: ", name);
            return 0;
        }

        return s;
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

        case ArgumentType::RoomScript: {
            const unsigned s = getRoomScriptId(value);
            data.push_back(s);
        } break;

        case ArgumentType::EntityGroup: {
            const unsigned g = getEntityGroupId(value);
            data.push_back(g);
        } break;

        case ArgumentType::Room: {
            const unsigned g = getRoomId(value);
            data.push_back(g);
        } break;

        case ArgumentType::RoomEntrance: {
            const unsigned g = getRoomEntranceId(value, room);
            data.push_back(g);
        } break;
        }
    }

    void roomAndRoomEntraceArguments(const std::array<std::string, 2>& arguments)
    {
        const unsigned roomId = getRoomId(arguments.at(0));
        unsigned entranceId = 0;

        if (roomId < roomsList.size()) {
            if (const auto* r = roomsList.at(roomId)) {
                entranceId = getRoomEntranceId(arguments.at(1), *r);
            }
        }

        data.push_back(roomId);
        data.push_back(entranceId);
    }

    void scriptArguments(const InstructionData& bc, const std::array<std::string, 2>& arguments, const size_t bytecodePos)
    {
        constexpr std::array<ArgumentType, 2> loadRoomArgs = { ArgumentType::Room, ArgumentType::RoomEntrance };

        if (bc.arguments == loadRoomArgs) {
            roomAndRoomEntraceArguments(arguments);
        }
        else {
            assert(bc.arguments.size() == arguments.size());
            for (const auto i : range(bc.arguments.size())) {
                statementArgument(bc.arguments.at(i), arguments.at(i), bytecodePos);
            }
        }
    }

    // Returns the position of the branch argument in the conditional statement
    unsigned conditional(const Conditional& condition)
    {
        switch (condition.type) {
        case ConditionalType::Word: {
            uint8_t opcode = bytecode.endScriptOpcode;
            const uint8_t wordId = getWordId(condition.variable);
            const uint16_t value = getImmediateU16(condition.value);

            switch (condition.comparison) {
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
            const unsigned flagId = getFlagId(condition.variable);

            switch (condition.comparison) {
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

        scriptArguments(bc, statement.arguments, bytecodePos);
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

    void operator()(const WhileStatement& whileStatement)
    {
        const unsigned loopPos = data.size();
        const unsigned branchPos = conditional(whileStatement.condition);

        if (whileStatement.statements.empty()) {
            addError("Empty body in an while statement");
        }

        compileStatements(whileStatement.statements);

        bool hasYieldInstruction = std::any_of(
            whileStatement.statements.begin(), whileStatement.statements.end(),
            [&](const ScriptNode& node) {
                if (auto* s = std::get_if<Statement>(&node)) {
                    auto it = bytecode.instructions.find(s->opcode);
                    if (it != bytecode.instructions.end()) {
                        return it->second.yields;
                    }
                }
                return false;
            });

        if (!hasYieldInstruction) {
            addWarning("while loop does not contain a yielding instruction, it may infiniately loop.");
        }

        // Loop back to start of while statement
        data.push_back(bytecode.gotoOpcode);
        data.push_back(loopPos);
        data.push_back(loopPos >> 8);

        writeWordAt(branchPos, data.size());
    }

    void operator()(const Comment&)
    {
        // Ignore comments
    }
};
}
