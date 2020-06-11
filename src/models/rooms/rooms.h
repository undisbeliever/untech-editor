/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/grid.h"
#include "models/common/idstring.h"
#include "models/common/namedlist.h"

namespace UnTech {
class ErrorList;
namespace Project {
template <typename T>
class DataStore;
}
namespace Entity {
struct CompiledEntityRomData;
}
namespace Resources {
struct CompiledScenesData;
}

namespace Rooms {

constexpr unsigned MAX_ROOM_ENTRANCES = 32;
constexpr unsigned MAX_ENTITY_GROUPS = 8;
constexpr unsigned MAX_ENTITY_ENTRIES = 96;

constexpr unsigned MAP_TILE_SIZE = 16;

// Vertical space above/below map where entity positon is still valid
constexpr int ENTITY_VERTICAL_SPACING = 256;

struct RoomSettings {
    constexpr static unsigned MIN_ROOM_DATA_SIZE = 1024;
    constexpr static unsigned MAX_ROOM_DATA_SIZE = 40 * 1024;

    unsigned roomDataSize = 16 * 1024;

    bool validate(ErrorList& err) const;

    bool operator==(const RoomSettings& o) const
    {
        return roomDataSize == o.roomDataSize;
    }
    bool operator!=(const RoomSettings& o) const { return !(*this == o); }
};

enum class RoomEntranceOrientation {
    DOWN_RIGHT,
    DOWN_LEFT,
    UP_RIGHT,
    UP_LEFT,
};

struct RoomEntrance {
    idstring name; // optional
    upoint position;
    RoomEntranceOrientation orientation;

    bool operator==(const RoomEntrance& o) const
    {
        return name == o.name
               && position == o.position
               && orientation == o.orientation;
    }
    bool operator!=(const RoomEntrance& o) const { return !(*this == o); }
};

struct EntityEntry {
    idstring name; // optional (to be used by scripting)

    idstring entityId;
    point position;

    // ::TODO add parameter (need to figure out parameter type and conversion to word)::

    bool operator==(const EntityEntry& o) const
    {
        return name == o.name
               && entityId == o.entityId
               && position == o.position;
    }
    bool operator!=(const EntityEntry& o) const { return !(*this == o); }
};

struct EntityGroup {
    idstring name; // required

    std::vector<EntityEntry> entities;

    bool operator==(const EntityGroup& o) const
    {
        return name == o.name
               && entities == o.entities;
    }
    bool operator!=(const EntityGroup& o) const { return !(*this == o); }
};

struct RoomInput {
    constexpr static unsigned MIN_MAP_WIDTH = 16;
    constexpr static unsigned MIN_MAP_HEIGHT = 14;

    constexpr static unsigned MAX_MAP_WIDTH = 255;
    constexpr static unsigned MAX_MAP_HEIGHT = 128;

    static const std::string FILE_EXTENSION;

    idstring name;

    idstring scene;

    grid<uint8_t> map;

    NamedList<RoomEntrance> entrances;
    NamedList<EntityGroup> entityGroups;

    rect validEntityArea() const;

    unsigned mapRight() const { return map.width() * MAP_TILE_SIZE; }
    unsigned mapBottom() const { return map.height() * MAP_TILE_SIZE; }

    bool validate(const Resources::CompiledScenesData& compiledScenes, ErrorList& err) const;

    bool operator==(const RoomInput& o) const
    {
        return name == o.name
               && scene == o.scene
               && map == o.map
               && entrances == o.entrances
               && entityGroups == o.entityGroups;
    }
    bool operator!=(const RoomInput& o) const { return !(*this == o); }
};

struct RoomData {
    const static int ROOM_FORMAT_VERSION;

    idstring name;
    std::vector<uint8_t> data;

    bool validate(ErrorList&) const { return data.empty() == false; }

    std::vector<uint8_t> exportSnesData() const;
};
std::unique_ptr<const RoomData> compileRoom(const RoomInput& input,
                                            const Resources::CompiledScenesData& compiledScenes,
                                            const Entity::CompiledEntityRomData& entityRomData,
                                            const RoomSettings& roomSettings,
                                            ErrorList& err);
}
}
