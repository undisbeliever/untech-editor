/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "bytecode.h"
#include "game-state.h"
#include "script.h"
#include "scripting-error.h"
#include "models/common/externalfilelist.h"
#include "models/common/iterators.h"
#include "models/common/string.h"
#include "models/rooms/rooms.h"

namespace UnTech::Scripting {

using GameStateDataValueMap = std::unordered_map<std::string, const GameStateData::Value>;

static inline std::unordered_map<std::string, unsigned>
setupTempVariables(const std::vector<idstring> tempVars, const bool isFlag,
                   const GameStateDataValueMap& gameStateVars, const unsigned nGameStateVars, const unsigned maxVars,
                   ErrorList& err)
{
    const char* typeName = isFlag ? "flag" : "word";
    const ScriptErrorType errorType = isFlag ? ScriptErrorType::TEMP_FLAG : ScriptErrorType::TEMP_WORD;

    auto addError = [&](unsigned index, const auto... msg) {
        err.addError(std::make_unique<ScriptError>(errorType, index,
                                                   stringBuilder("Temporary script ", typeName, ": ", msg...)));
    };

    std::unordered_map<std::string, unsigned> map;
    map.reserve(tempVars.size());

    long varId = maxVars;

    for (auto [index, varName] : enumerate(tempVars)) {
        varId--;

        if (!varName.isValid()) {
            addError(index, "Missing name");
        }
        else if (gameStateVars.find(varName.str()) != gameStateVars.end()) {
            addError(index, "Name exists in GameState: ", varName);
        }
        else {
            const auto [it, inserted] = map.emplace(varName.str(), varId);
            if (!inserted) {
                addError(index, "Duplicate name detected");
            }
        }
    }

    if (tempVars.size() + nGameStateVars > maxVars) {
        addError(INT_MAX, "Too many temporary script ", typeName, " variables");
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
    unsigned scriptIndex;

    unsigned lineNo;
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
        , tempFlags(setupTempVariables(room.roomScripts.tempFlags, true, gameState.flags, gameState.nFlags, GameState::MAX_FLAGS, e))
        , tempWords(setupTempVariables(room.roomScripts.tempWords, false, gameState.words, gameState.nWords, GameState::MAX_WORDS, e))
        , data(data)
        , scriptIndex(0)
        , lineNo(0)
        , depth(0)
        , err(e)
        , valid(true)
    {
        valid &= tempFlags.size() == room.roomScripts.tempFlags.size();
        valid &= tempWords.size() == room.roomScripts.tempWords.size();
    }

    bool isValid() const { return valid; }

    unsigned compileScript(const Script& script, const unsigned index)
    {
        const unsigned scriptPos = data.size();

        scriptName = script.name;
        scriptIndex = index;
        lineNo = 0;

        if (script.name == STARTUP_SCRIPT_NAME) {
            if (&script != &room.roomScripts.startupScript) {
                addScriptError("Invalid script name");
            }
        }

        compileStatements(script.statements);

        data.push_back(bytecode.endScriptOpcode);

        assert(depth == 0);

        return scriptPos;
    }

private:
    template <typename... T>
    void addScriptError(T... msg)
    {
        err.addError(std::make_unique<ScriptError>(ScriptErrorType::SCRIPT, scriptIndex,
                                                   stringBuilder("Script ", scriptName, ": ", msg...)));
        valid = false;
    }

    template <typename... T>
    void addLineError(T... msg)
    {
        err.addError(std::make_unique<ScriptError>(ScriptErrorType::SCRIPT_LINE, scriptIndex, lineNo,
                                                   stringBuilder("Script ", scriptName, " (line ", lineNo, "): ", msg...)));
        valid = false;
    }

    template <typename... T>
    void addLineWarning_lineNo(unsigned wLineNo, T... msg)
    {
        err.addWarning(std::make_unique<ScriptError>(ScriptErrorType::SCRIPT_LINE, scriptIndex, wLineNo,
                                                     stringBuilder("Script ", scriptName, " (line ", wLineNo, "): ", msg...)));

        valid = false;
    }

    void compileStatements(const std::vector<ScriptNode>& statements)
    {
        depth++;
        if (depth > Script::MAX_DEPTH) {
            addScriptError("Script too deep");
            return;
        }

        for (auto& s : statements) {
            lineNo++;
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
                addLineError("Invalid flag `", name, "` (it is private and belongs to room `", room.name, "`)");
            }
            return it->second.index;
        }

        addLineError("Unknown flag: ", name);
        return 0;
    }
    unsigned getFlagId(const idstring& name) { return getFlagId(name.str()); }

    unsigned getWordId(const std::string& name)
    {
        auto localIt = tempWords.find(name);
        if (localIt != tempWords.end()) {
            return localIt->second;
        }

        auto it = gameState.words.find(name);
        if (it != gameState.words.end()) {
            if (!it->second.allowedInRoom(room.name)) {
                addLineError("Invalid word `", name, "` (it is private and belongs to room `", room.name, "`)");
            }
            return it->second.index;
        }

        addLineError("Unknown word: ", name);
        return 0;
    }
    unsigned getWordId(const idstring& name) { return getWordId(name.str()); }

    unsigned getImmediateU16(const std::string& value)
    {
        auto u16 = String::decimalOrHexToUint16(value);

        if (u16) {
            return *u16;
        }
        else {
            addLineError("Invalid argument: ", value);
            return 0;
        }
    }

    unsigned getRoomScriptId(const std::string& value)
    {
        const auto name = idstring::fromString(value);
        if (!name.isValid()) {
            addLineError("Invalid script name: ", value);
            return 0;
        }

        const auto s = room.roomScripts.scripts.indexOf(name);

        if (s > room.roomScripts.scripts.size()) {
            addLineError("Unknown script: ", name);
            return 0;
        }

        return s;
    }

    unsigned getEntityGroupId(const std::string& value)
    {
        const auto name = idstring::fromString(value);
        if (!name.isValid()) {
            addLineError("Invalid entity group name: ", value);
            return 0;
        }

        const auto s = room.entityGroups.indexOf(name);

        if (s > room.entityGroups.size()) {
            addLineError("Unknown entity group: ", name);
            return 0;
        }

        return s;
    }

    unsigned getRoomId(const std::string& value)
    {
        const auto name = idstring::fromString(value);
        if (!name.isValid()) {
            addLineError("Invalid room name: ", value);
            return 0;
        }

        const auto s = roomsList.indexOf(name);

        if (s > roomsList.size()) {
            addLineError("Unknown room: ", name);
            return 0;
        }

        return s;
    }

    unsigned getRoomEntranceId(const std::string& value, const Rooms::RoomInput& r)
    {
        const auto name = idstring::fromString(value);
        if (!name.isValid()) {
            addLineError("Invalid room entrance name: ", value);
            return 0;
        }

        const auto s = r.entrances.indexOf(name);

        if (s > r.entrances.size()) {
            addLineError("Unknown room entrance: ", name);
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
            if (const auto r = roomsList.at(roomId)) {
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
                addLineError("Invalid comparison type for word variable");
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
                addLineError("Invalid comparison type for flag variable");
                break;
            }

            data.push_back(opcode);
            data.push_back(flagId & 0xff);
            data.push_back(0);
            data.push_back(0);
            return data.size() - 2;
        }
        }

        throw invalid_argument("Invalid ConditionalType");
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
            addLineError("Unknown instruction: ", statement.opcode);
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
            addLineError("Empty body in an if statement");
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
            addLineError("Empty body in an while statement");
        }

        const unsigned whileLineNo = lineNo;

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
            addLineWarning_lineNo(whileLineNo, "while loop does not contain a yielding instruction, it may infiniately loop.");
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
