/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/errorlist.h"
#include "models/common/idstring.h"
#include <memory>
#include <unordered_map>
#include <vector>

namespace UnTech {
template <class T>
class ExternalFileList;
}

namespace UnTech::Entity {
struct EntityRomData;
}

namespace UnTech::Rooms {
struct RoomInput;
}

namespace UnTech::Scripting {

struct GameStateFlag {
    idstring name; // Is allowed to be empty
    idstring room; // If empty then flag is global

    bool operator==(const GameStateFlag& o) const
    {
        return name == o.name
               && room == o.room;
    }
};

struct GameStateWord {
    idstring name; // Is allowed to be empty
    idstring room; // If empty then flag is global
    uint16_t initialValue = 0;

    bool operator==(const GameStateWord& o) const
    {
        return name == o.name
               && room == o.room
               && initialValue == o.initialValue;
    }
};

struct GameState {
    constexpr static unsigned MAX_FLAGS = 512;
    constexpr static unsigned MAX_WORDS = 128;

    idstring startingRoom;
    idstring startingEntrance;
    idstring startingPlayer;

    std::vector<GameStateFlag> flags;
    std::vector<GameStateWord> words;

    bool operator==(const GameState& o) const
    {
        return startingRoom == o.startingRoom
               && startingEntrance == o.startingEntrance
               && startingPlayer == o.startingPlayer
               && flags == o.flags
               && words == o.words;
    }
};

struct GameStateData {
    static const int GAME_STATE_FORMAT_VERSION;

    struct Value {
        unsigned index;
        idstring room;

        bool allowedInRoom(const idstring& r) const
        {
            return !room.isValid() || r == room;
        }
    };

    std::unordered_map<idstring, const Value> flags;
    std::unordered_map<idstring, const Value> words;

    unsigned nFlags;
    unsigned nWords;

    std::vector<uint8_t> initialGameState;

    std::vector<uint8_t> exportSnesData() const;
};

std::shared_ptr<const GameStateData>
compileGameState(const GameState& input,
                 const ExternalFileList<Rooms::RoomInput>& rooms, const Entity::EntityRomData& entityRomData,
                 ErrorList& err);

void writeGameStateConstants(const GameState& input, const GameStateData& inputData, std::ostream& out);

}
