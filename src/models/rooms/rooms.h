/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/grid.h"
#include "models/common/idstring.h"
#include "models/common/namedlist.h"
#include "models/scripting/script.h"

namespace UnTech {
class ErrorList;

template <typename T>
class ExternalFileList;
}

namespace UnTech::Project {
template <typename T>
class DataStore;
}

namespace UnTech::Entity {
struct CompiledEntityRomData;
}

namespace UnTech::Resources {
struct CompiledScenesData;
}

namespace UnTech::MetaTiles {
struct MetaTileTilesetData;
}

namespace UnTech::Rooms {

constexpr unsigned MAX_ROOM_ENTRANCES = 32;
constexpr unsigned MAX_ENTITY_GROUPS = 8;
constexpr unsigned MAX_ENTITY_ENTRIES = 96;
constexpr unsigned MAX_N_SCRIPTS = 16;
constexpr unsigned MAX_SCRIPT_TRIGGERS = 16;

constexpr unsigned MAP_TILE_SIZE = 16;

// Vertical space above/below map where entity positon is still valid
constexpr int ENTITY_VERTICAL_SPACING = 256;

struct RoomSettings {
    constexpr static unsigned MIN_ROOM_DATA_SIZE = 1024;
    constexpr static unsigned MAX_ROOM_DATA_SIZE = 40 * 1024;

    unsigned roomDataSize = 16 * 1024;

    bool operator==(const RoomSettings&) const = default;
};

bool validate(const RoomSettings& input, ErrorList& err);

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

    bool operator==(const RoomEntrance&) const = default;
};

struct EntityEntry {
    idstring name; // optional (to be used by scripting)

    idstring entityId;
    point position;

    std::string parameter;

    bool operator==(const EntityEntry&) const = default;
};

struct EntityGroup {
    idstring name; // required

    std::vector<EntityEntry> entities;

    bool operator==(const EntityGroup&) const = default;
};

struct ScriptTrigger {
    idstring script;
    urect aabb;

    bool once;

    bool operator==(const ScriptTrigger&) const = default;
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

    Scripting::RoomScripts roomScripts;
    std::vector<ScriptTrigger> scriptTriggers;

    rect validEntityArea() const;

    unsigned mapRight() const { return map.width() * MAP_TILE_SIZE; }
    unsigned mapBottom() const { return map.height() * MAP_TILE_SIZE; }

    unsigned tileIndex(const upoint& p) const;

    bool operator==(const RoomInput&) const = default;
};

struct RoomData {
    const static int ROOM_FORMAT_VERSION;

    std::vector<uint8_t> data;

    std::vector<uint8_t> exportSnesData() const;
};

std::shared_ptr<const RoomData>
compileRoom(const RoomInput& input, const ExternalFileList<RoomInput>& roomsList,
            const Resources::CompiledScenesData& compiledScenes, const Entity::CompiledEntityRomData& entityRomData, const RoomSettings& roomSettings,
            const Scripting::GameStateData& gameStateData, const Scripting::BytecodeMapping& bytecodeData,
            const Project::DataStore<MetaTiles::MetaTileTilesetData>& metaTilesData,
            ErrorList& err);
}
