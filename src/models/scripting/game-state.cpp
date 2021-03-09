/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "game-state.h"
#include "models/common/externalfilelist.h"
#include "models/common/iterators.h"
#include "models/entity/entityromdata.h"
#include "models/lz4/lz4.h"
#include "models/rooms/rooms.h"

namespace UnTech::Scripting {

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

    if (input.flags.size() > GameState::MAX_FLAGS) {
        addError("Too many flags (", input.flags.size(), ", max: ", GameState::MAX_FLAGS, ")");
    }
    if (input.words.size() > GameState::MAX_WORDS) {
        addError("Too many words (", input.words.size(), ", max: ", GameState::MAX_WORDS, ")");
    }

    const auto roomId = rooms.indexOf(input.startingRoom);
    unsigned entranceId = INT_MAX;
    if (roomId < rooms.size()) {
        if (const auto* room = rooms.at(roomId)) {
            entranceId = room->entrances.indexOf(input.startingEntrance);
        }
    }

    if (!input.startingRoom.isValid()) {
        addError("Missing startingRoom");
    }
    else if (!input.startingEntrance.isValid()) {
        addError("Missing startingEntrance");
    }
    else if (roomId > UINT8_MAX) {
        addError("Cannot find room: ", input.startingRoom);
    }
    else if (entranceId > UINT8_MAX) {
        addError("Cannot find room entrance: ", input.startingEntrance);
    }

    const unsigned playerId = entityRomData.players.indexOf(input.startingPlayer);
    if (playerId > UINT8_MAX) {
        addError("Cannot find player: ", input.startingPlayer);
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

                const auto [it, inserted] = map.emplace(item.name, GameStateData::Value{ unsigned(i), item.room });
                if (!inserted) {
                    addError("Duplicate ", typeName, " detected: ", item.name);
                }
            }
        }

        return lastIndex + 1;
    };
    ret->nFlags = processList(ret->flags, input.flags, "flag");
    ret->nWords = processList(ret->words, input.words, "word");

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

    if (!valid) {
        ret = nullptr;
    }

    return ret;
}
const int GameStateData::GAME_STATE_FORMAT_VERSION = 1;

std::vector<uint8_t> GameStateData::exportSnesData() const
{
    return lz4HcCompress(initialGameState);
}

void writeGameStateConstants(const GameState& input, const GameStateData& inputData, std::ostream& out)
{
    out << "constant Project.GameState.N_FLAGS = " << inputData.nFlags
        << "\nconstant Project.GameState.MAX_FLAGS = " << input.MAX_FLAGS
        << "\nconstant Project.GameState.N_WORDS = " << inputData.nWords
        << "\nconstant Project.GameState.MAX_WORDS = " << input.MAX_WORDS
        << "\n"
           "\n"
           "namespace Project.GameState.Words {";

    for (auto [i, word] : const_enumerate(input.words)) {
        // Only add global words to game state constants
        if (word.name.isValid() && !word.room.isValid()) {
            out << "\n  constant " << word.name << " = GameState.wordData + " << (i * 2);
        }
    }

    out << "\n}\n\n";
}

}
