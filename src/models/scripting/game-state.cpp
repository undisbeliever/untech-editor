/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "game-state.h"
#include "scripting-error.h"
#include "models/common/externalfilelist.h"
#include "models/common/iterators.h"
#include "models/common/stringstream.h"
#include "models/entity/entityromdata.h"
#include "models/lz4/lz4.h"
#include "models/rooms/rooms.h"

namespace UnTech::Scripting {

template <typename... Args>
std::unique_ptr<GameStateError> variableError(const GameStateFlag& flag, const unsigned index, const Args... msg)
{
    return std::make_unique<GameStateError>(GameStateErrorType::FLAG, index,
                                            stringBuilder(u8"Flag ", flag.name, u8": ", msg...));
}

template <typename... Args>
std::unique_ptr<GameStateError> variableError(const GameStateWord& word, const unsigned index, const Args... msg)
{
    return std::make_unique<GameStateError>(GameStateErrorType::WORD, index,
                                            stringBuilder(u8"Word ", word.name, u8": ", msg...));
}

std::shared_ptr<const GameStateData>
compileGameState(const GameState& input,
                 const ExternalFileList<Rooms::RoomInput>& rooms, const Entity::EntityRomData& entityRomData,
                 ErrorList& err)
{
    bool valid = true;
    auto addError = [&](const auto... msg) {
        err.addErrorString(msg...);
        valid = false;
    };
    auto addVariableError = [&](const auto& variable, const unsigned index, const auto... msg) {
        err.addError(variableError(variable, index, msg...));
        valid = false;
    };

    if (input.flags.size() > GameState::MAX_FLAGS) {
        addError(u8"Too many flags (", input.flags.size(), u8", max: ", GameState::MAX_FLAGS, u8")");
    }
    if (input.words.size() > GameState::MAX_WORDS) {
        addError(u8"Too many words (", input.words.size(), u8", max: ", GameState::MAX_WORDS, u8")");
    }

    const auto roomId = rooms.indexOf(input.startingRoom);
    unsigned entranceId = INT_MAX;
    if (roomId < rooms.size()) {
        if (const auto room = rooms.at(roomId)) {
            entranceId = room->entrances.indexOf(input.startingEntrance);
        }
    }

    if (!input.startingRoom.isValid()) {
        addError(u8"Missing startingRoom");
    }
    else if (!input.startingEntrance.isValid()) {
        addError(u8"Missing startingEntrance");
    }
    else if (roomId > UINT8_MAX) {
        addError(u8"Cannot find room: ", input.startingRoom);
    }
    else if (entranceId > UINT8_MAX) {
        addError(u8"Cannot find room entrance: ", input.startingEntrance);
    }

    const unsigned playerId = entityRomData.players.indexOf(input.startingPlayer);
    if (playerId > UINT8_MAX) {
        addError(u8"Cannot find player: ", input.startingPlayer);
    }

    if (!valid) {
        return nullptr;
    }

    auto ret = std::make_shared<GameStateData>();

    auto processList = [&](auto& map, const auto& list, const auto typeName) {
        map.reserve(list.size());

        unsigned lastIndex = 0;

        for (auto [i, item] : const_enumerate(list)) {
            if (item.name.isValid()) {
                lastIndex = i;

                const auto [it, inserted] = map.emplace(item.name.str(), GameStateData::Value{ unsigned(i), item.room });
                if (!inserted) {
                    addVariableError(item, i, u8"Duplicate ", typeName, u8" detected");
                }
            }
        }

        return lastIndex + 1;
    };
    ret->nFlags = processList(ret->flags, input.flags, u8"flag");
    ret->nWords = processList(ret->words, input.words, u8"word");

    {
        static_assert(2 * GameState::MAX_WORDS == 0x100);

        ret->initialGameState.resize(2 * GameState::MAX_WORDS + 3);

        auto it = ret->initialGameState.begin();

        for (const auto& w : input.words) {
            *it++ = w.initialValue & 0xff;
            *it++ = (w.initialValue >> 8) & 0xff;
        }
        assert(it <= ret->initialGameState.begin() + 2 * GameState::MAX_WORDS);

        it = ret->initialGameState.begin() + 2 * GameState::MAX_WORDS;

        *it++ = roomId;
        *it++ = entranceId;
        *it++ = playerId;

        assert(it == ret->initialGameState.end());
    }

    assert(valid);

    return ret;
}
const int GameStateData::GAME_STATE_FORMAT_VERSION = 1;

std::vector<uint8_t> GameStateData::exportSnesData() const
{
    return lz4HcCompress(initialGameState);
}

void writeGameStateConstants(const GameState& input, const GameStateData& inputData, StringStream& out)
{
    out.write(u8"constant Project.GameState.N_FLAGS = ", inputData.nFlags,
              u8"\nconstant Project.GameState.MAX_FLAGS = ", input.MAX_FLAGS,
              u8"\nconstant Project.GameState.N_WORDS = ", inputData.nWords,
              u8"\nconstant Project.GameState.MAX_WORDS = ", input.MAX_WORDS,
              u8"\n"
              u8"\n"
              u8"namespace Project.GameState.Words {");

    for (auto [i, word] : const_enumerate(input.words)) {
        // Only add global words to game state constants
        if (word.name.isValid() && !word.room.isValid()) {
            out.write(u8"\n  constant ", word.name, u8" = GameState.wordData + ", (i * 2));
        }
    }

    out.write(u8"\n}\n\n");
}

}
