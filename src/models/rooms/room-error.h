/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/errorlist.h"

namespace UnTech::Rooms {

enum class RoomErrorType {
    ROOM_ENTRANCE,
    ENTITY_GROUP,
    ENTITY_ENTRY,
    SCRIPT_TRIGGER,
};

class RoomError : public GenericListError {
public:
    const RoomErrorType type;

    RoomError(const RoomErrorType type, unsigned pIndex, std::string&& message)
        : GenericListError(pIndex, std::move(message))
        , type(type)
    {
    }

    RoomError(const RoomErrorType type, unsigned pIndex, unsigned cIndex, std::string&& message)
        : GenericListError(pIndex, cIndex, std::move(message))
        , type(type)
    {
    }
};

struct InvalidRoomTile {
    unsigned x;
    unsigned y;
    unsigned reasonBits;

    constexpr static uint8_t NO_EMPTY_TILE_ABOVE_DOWN_SLOPE = 1;
    constexpr static uint8_t NO_FLOOR_BELOW_DOWN_SLOPE = 2;
    constexpr static uint8_t NO_CEILING_ABOVE_UP_SLOPE = 4;
    constexpr static uint8_t NO_EMPTY_TILE_BELOW_UP_SLOPE = 8;
    constexpr static uint8_t INVALID_TILE_ON_THE_LEFT = 16;
    constexpr static uint8_t INVALID_TILE_ON_THE_RIGHT = 32;
    constexpr static uint8_t SLOPE_ON_BORDER = 64;

    InvalidRoomTile(unsigned x, unsigned y, uint8_t direction)
        : x(x)
        , y(y)
        , reasonBits(direction)
    {
    }
};

class InvalidRoomTilesError : public AbstractError {
public:
    const std::vector<InvalidRoomTile> invalidTiles;

public:
    InvalidRoomTilesError(std::vector<InvalidRoomTile>&& invalidTiles);
    virtual ~InvalidRoomTilesError();

    virtual void printIndented(StringStream& out) const final;
};

}
