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

}
