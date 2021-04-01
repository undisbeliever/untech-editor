/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "room-error.h"
#include "rooms.h"

namespace UnTech::Rooms {

template <typename... Args>
inline std::unique_ptr<RoomError> roomEntranceError(const RoomEntrance& e, const unsigned index, const Args... msg)
{
    return std::make_unique<RoomError>(RoomErrorType::ROOM_ENTRANCE, index,
                                       stringBuilder("Entity Entrance ", e.name, ": ", msg...));
}

template <typename... Args>
inline std::unique_ptr<RoomError> entityGroupError(const EntityGroup& eg, const unsigned index, const Args... msg)
{
    return std::make_unique<RoomError>(RoomErrorType::ENTITY_GROUP, index,
                                       stringBuilder("Entity Group ", eg.name, ": ", msg...));
}

template <typename... Args>
inline std::unique_ptr<RoomError> entityEntryError(const EntityGroup& eg, const unsigned egIndex, const unsigned eeIndex, const Args... msg)
{
    return std::make_unique<RoomError>(RoomErrorType::ENTITY_ENTRY, egIndex, eeIndex,
                                       stringBuilder("Entity Group ", eg.name, " Entry #", eeIndex, ": ", msg...));
}

template <typename... Args>
inline std::unique_ptr<RoomError> scriptTriggerError(const unsigned index, const Args... msg)
{
    return std::make_unique<RoomError>(RoomErrorType::SCRIPT_TRIGGER, index,
                                       stringBuilder("Script Trigger ", index, ": ", msg...));
}

}
