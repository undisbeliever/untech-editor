/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "rooms.h"
#include "errorlisthelpers.h"
#include "models/common/errorlist.h"
#include "models/common/iterators.h"
#include "models/common/string.h"
#include "models/common/validateunique.h"
#include "models/entity/entityromdata.h"
#include "models/lz4/lz4.h"
#include "models/metatiles/common.h"
#include "models/metatiles/metatile-tileset.h"
#include "models/project/project-data.h"
#include "models/resources/scenes.h"
#include "models/scripting/script-compiler.hpp"
#include <algorithm>

namespace UnTech::Rooms {

constexpr static unsigned MAP_HEIGHT_SMALL = 64;
constexpr static unsigned MAP_HEIGHT_LARGE = 128;

constexpr static unsigned MAP_ORIGIN_X = 0x1000;
constexpr static unsigned MAP_ORIGIN_Y = 0x1000;

constexpr static unsigned X_POSITION_MASK = (MAP_ORIGIN_X - 1);

constexpr static unsigned Y_POSITION_MASK_SMALL = (MAP_HEIGHT_SMALL - 1) * MAP_TILE_SIZE;
constexpr static unsigned Y_POSITION_MASK_LARGE = (MAP_HEIGHT_LARGE - 1) * MAP_TILE_SIZE;

// Ensure entities X position can masked safely
static_assert(UINT8_MAX * MAP_TILE_SIZE < X_POSITION_MASK);

// Ensure entities Y position is always within the Y-Position MASK
static_assert(Y_POSITION_MASK_SMALL < MAP_ORIGIN_Y - ENTITY_VERTICAL_SPACING);
static_assert(Y_POSITION_MASK_LARGE < MAP_ORIGIN_Y - ENTITY_VERTICAL_SPACING);
static_assert(Y_POSITION_MASK_SMALL < MAP_ORIGIN_Y + ENTITY_VERTICAL_SPACING);
static_assert(Y_POSITION_MASK_LARGE < MAP_ORIGIN_Y + ENTITY_VERTICAL_SPACING);

// Ensure map can be read/written safely
static_assert(RoomInput::MAX_MAP_HEIGHT == MAP_HEIGHT_LARGE);
static_assert(RoomInput::MAX_MAP_HEIGHT <= MetaTiles::MAX_GRID_HEIGHT);
static_assert(RoomInput::MAX_MAP_WIDTH <= MetaTiles::MAX_GRID_WIDTH);

bool validate(const RoomSettings& input, ErrorList& err)
{
    bool valid = true;

    auto validateMinMax = [&](unsigned value, unsigned min, unsigned max, const char* msg) {
        if (value < min || value > max) {
            err.addErrorString(msg, " (", value, ", min: ", min, ", max: ", max, ")");
            valid = false;
        }
    };

    validateMinMax(input.roomDataSize, input.MIN_ROOM_DATA_SIZE, input.MAX_ROOM_DATA_SIZE, "Max Room Size invalid");

    return valid;
}

static bool validateEntrances(const NamedList<RoomEntrance>& entrances, const RoomInput& ri, ErrorList& err)
{
    bool valid = true;

    const auto addError = [&](const auto... msg) {
        err.addErrorString(msg...);
        valid = false;
    };
    const auto addEntranceError = [&](const RoomEntrance& e, const unsigned index, const auto... msg) {
        err.addError(roomEntranceError(e, index, "Entity Entrance ", e.name, ": ", msg...));
        valid = false;
    };

    if (entrances.empty()) {
        addError("Expected at least one Room Entrance");
    }

    if (entrances.size() > MAX_ROOM_ENTRANCES) {
        addError("Too many Room Entrances (", entrances.size(), ", max: ", MAX_ROOM_ENTRANCES, ")");
    }

    valid &= validateNamesUnique(entrances, "Room Entrance", [&](unsigned i, auto... msg) {
        err.addError(std::make_unique<RoomError>(RoomErrorType::ROOM_ENTRANCE, i, stringBuilder(msg...)));
    });

    for (auto [i, en] : enumerate(entrances)) {
        if (en.position.x >= ri.mapRight() || en.position.y >= ri.mapBottom()) {
            addEntranceError(en, i, "Entrance must be inside map");
        }
    }

    return valid;
}

static bool validateEntityGroups(const NamedList<EntityGroup>& entityGroups, const RoomInput& ri, ErrorList& err)
{
    bool valid = true;

    // EntityGroup/EntityEntry names are checked for uniqueness in the compiler

    const auto addError = [&](const auto... msg) {
        err.addErrorString(msg...);
        valid = false;
    };
    const auto addEntityGroupError = [&](const EntityGroup& eg, const unsigned egIndex, const auto... msg) {
        err.addError(entityGroupError(eg, egIndex, msg...));
        valid = false;
    };
    const auto addEntityError = [&](const EntityGroup& eg, const unsigned egIndex, const unsigned eeIndex, const auto... msg) {
        err.addError(entityEntryError(eg, egIndex, eeIndex, msg...));
        valid = false;
    };

    if (entityGroups.size() > MAX_ENTITY_GROUPS) {
        addError("Too many Entity Groups (", entityGroups.size(), ", max: ", MAX_ENTITY_GROUPS, ")");
    }

    const rect mapBounds = ri.validEntityArea();

    unsigned count = 0;
    for (auto [egIndex, eg] : enumerate(entityGroups)) {
        if (eg.name.isValid() == false) {
            addEntityGroupError(eg, egIndex, "Expected name");
        }

        if (eg.entities.empty()) {
            addEntityGroupError(eg, egIndex, "Expected at least one entity");
        }

        for (const auto [eeIndex, ee] : enumerate(eg.entities)) {
            if (mapBounds.contains(ee.position) == false) {
                addEntityError(eg, egIndex, eeIndex, "Entity outside of map bounds");
            }
            count++;
        }
    }

    if (count > MAX_ENTITY_ENTRIES) {
        addError("Too many Entities (", count, ", max: ", MAX_ENTITY_ENTRIES, ")");
    }

    return valid;
}

static bool validateScriptTriggers(const std::vector<ScriptTrigger>& scriptTriggers, const RoomInput& room, ErrorList& err)
{
    bool valid = true;

    // EntityGroup/EntityEntry names are checked for uniqueness in the compiler

    const auto addError = [&](const auto... msg) {
        err.addErrorString(msg...);
        valid = false;
    };
    const auto addStError = [&](unsigned i, const auto... msg) {
        err.addError(scriptTriggerError(i, msg...));
        valid = false;
    };

    if (scriptTriggers.size() > MAX_SCRIPT_TRIGGERS) {
        addError("Too many Entity Groups (", scriptTriggers.size(), ", max: ", MAX_SCRIPT_TRIGGERS, ")");
    }

    for (auto [i, st] : const_enumerate(scriptTriggers)) {
        if (st.script.isValid()) {
            if (!room.roomScripts.scripts.find(st.script)) {
                addStError(i, "Cannot find script: ", st.script);
            }
        }
        else {
            addStError(i, "Expected script");
        }

        if (st.aabb.width <= 0 || st.aabb.height <= 0) {
            addStError(i, "Invalid AABB");
        }
        if (!room.map.size().contains(st.aabb)) {
            addStError(i, "AABB must be inside the room");
        }
    }

    return valid;
}

rect RoomInput::validEntityArea() const
{
    return {
        0, -ENTITY_VERTICAL_SPACING,
        map.width() * MAP_TILE_SIZE, map.height() * MAP_TILE_SIZE + ENTITY_VERTICAL_SPACING * 2
    };
}

unsigned RoomInput::tileIndex(const upoint& p) const
{
    if (map.height() <= MAP_HEIGHT_SMALL) {
        return p.x * MAP_HEIGHT_SMALL + p.y;
    }
    else {
        return p.x * MAP_HEIGHT_LARGE + p.y;
    }
}

static bool isTileAllowedLeftofEndSlope(const MetaTiles::TileCollisionType tile)
{
    using TCT = MetaTiles::TileCollisionType;

    switch (tile) {
    case TCT::DOWN_RIGHT_SLOPE:
    case TCT::DOWN_RIGHT_TALL_SLOPE:
    case TCT::UP_RIGHT_TALL_SLOPE:
    case TCT::UP_RIGHT_SLOPE:
        return true;

    case TCT::END_SLOPE:
    case TCT::SOLID:
        return true;

    case TCT::DOWN_LEFT_SLOPE:
    case TCT::DOWN_RIGHT_SHORT_SLOPE:
    case TCT::DOWN_LEFT_TALL_SLOPE:
    case TCT::DOWN_LEFT_SHORT_SLOPE:
    case TCT::UP_LEFT_SLOPE:
    case TCT::UP_RIGHT_SHORT_SLOPE:
    case TCT::UP_LEFT_TALL_SLOPE:
    case TCT::UP_LEFT_SHORT_SLOPE:
        return false;

    case TCT::EMPTY:
    case TCT::UP_PLATFORM:
    case TCT::DOWN_PLATFORM:
        return false;
    }

    return false;
}

static bool isTileAllowedRightofEndSlope(const MetaTiles::TileCollisionType tile)
{
    using TCT = MetaTiles::TileCollisionType;

    switch (tile) {
    case TCT::DOWN_LEFT_SLOPE:
    case TCT::DOWN_LEFT_TALL_SLOPE:
    case TCT::UP_LEFT_TALL_SLOPE:
    case TCT::UP_LEFT_SLOPE:
        return true;

    case TCT::END_SLOPE:
    case TCT::SOLID:
        return true;

    case TCT::DOWN_RIGHT_SLOPE:
    case TCT::DOWN_RIGHT_SHORT_SLOPE:
    case TCT::DOWN_RIGHT_TALL_SLOPE:
    case TCT::DOWN_LEFT_SHORT_SLOPE:
    case TCT::UP_RIGHT_SLOPE:
    case TCT::UP_RIGHT_SHORT_SLOPE:
    case TCT::UP_RIGHT_TALL_SLOPE:
    case TCT::UP_LEFT_SHORT_SLOPE:
        return false;

    case TCT::EMPTY:
    case TCT::UP_PLATFORM:
    case TCT::DOWN_PLATFORM:
        return false;
    }

    return false;
}

static unsigned checkTileCollision_Border(const unsigned x, const unsigned y,
                                          const grid<uint8_t>& map, const MetaTiles::MetaTileTilesetData& tileset)
{
    unsigned ret = 0;

    auto getTc = [&](unsigned x, unsigned y) {
        return tileset.tileCollisions.at(map.at(x, y));
    };

    using TCT = MetaTiles::TileCollisionType;

    const TCT tile = getTc(x, y);

    switch (tile) {
    case TCT::DOWN_RIGHT_SLOPE:
    case TCT::DOWN_LEFT_SLOPE:
    case TCT::DOWN_RIGHT_SHORT_SLOPE:
    case TCT::DOWN_RIGHT_TALL_SLOPE:
    case TCT::DOWN_LEFT_TALL_SLOPE:
    case TCT::DOWN_LEFT_SHORT_SLOPE:
    case TCT::UP_RIGHT_SLOPE:
    case TCT::UP_LEFT_SLOPE:
    case TCT::UP_RIGHT_SHORT_SLOPE:
    case TCT::UP_RIGHT_TALL_SLOPE:
    case TCT::UP_LEFT_TALL_SLOPE:
    case TCT::UP_LEFT_SHORT_SLOPE:
        ret |= InvalidRoomTile::SLOPE_ON_BORDER;
        break;

    case TCT::END_SLOPE:
        if (x > 0) {
            const TCT left = getTc(x - 1, y);

            if (!isTileAllowedLeftofEndSlope(left)) {
                ret |= InvalidRoomTile::INVALID_TILE_ON_THE_LEFT;
            }
        }
        if (x < map.width() - 1) {
            const TCT right = getTc(x + 1, y);

            if (!isTileAllowedRightofEndSlope(right)) {
                ret |= InvalidRoomTile::INVALID_TILE_ON_THE_RIGHT;
            }
        }
        break;

    case TCT::EMPTY:
    case TCT::SOLID:
    case TCT::UP_PLATFORM:
    case TCT::DOWN_PLATFORM:
        break;
    }

    return ret;
}

// NOTE: This function MUST NOT be called on a tile along the border of the map grid.
static unsigned checkTileCollision_NotBorder(const unsigned x, const unsigned y,
                                             const grid<uint8_t>& map, const MetaTiles::MetaTileTilesetData& tileset)
{
    using TCT = MetaTiles::TileCollisionType;

    unsigned ret = 0;

    auto getTc = [&](unsigned x, unsigned y) {
        return tileset.tileCollisions.at(map.at(x, y));
    };

    const TCT tile = getTc(x, y);

    if (tile == TCT::EMPTY
        || tile == TCT::SOLID
        || tile == TCT::UP_PLATFORM
        || tile == TCT::DOWN_PLATFORM) {

        return 0;
    }

    const TCT above = getTc(x, y - 1);
    const TCT below = getTc(x, y + 1);

    const TCT left = getTc(x - 1, y);
    const TCT right = getTc(x + 1, y);

    auto testDownSlopeAboveBelow = [&]() {
        // ::TODO add configurable minimum slope spacing::
        if (above != TCT::EMPTY) {
            ret |= InvalidRoomTile::NO_EMPTY_TILE_ABOVE_DOWN_SLOPE;
        }
        if (below != TCT::SOLID && below != TCT::END_SLOPE) {
            ret |= InvalidRoomTile::NO_FLOOR_BELOW_DOWN_SLOPE;
        }
    };

    auto testUpSlopeAboveBelow = [&]() {
        // ::TODO add configurable minimum slope spacing::
        if (below != TCT::EMPTY) {
            ret |= InvalidRoomTile::NO_EMPTY_TILE_BELOW_UP_SLOPE;
        }
        if (above != TCT::SOLID && above != TCT::END_SLOPE) {
            ret |= InvalidRoomTile::NO_CEILING_ABOVE_UP_SLOPE;
        }
    };

    auto testDownSlopeLeft = [&](const auto... args) {
        if (left == TCT::SOLID) {
            // A SOLID tile is valid if it is a part of a wall
            const TCT aboveLeft = getTc(x - 1, y - 1);
            if (aboveLeft != TCT::SOLID) {
                ret |= InvalidRoomTile::INVALID_TILE_ON_THE_LEFT;
            }
        }
        else if ((... && (left != args))) {
            ret |= InvalidRoomTile::INVALID_TILE_ON_THE_LEFT;
        }
    };

    auto testUpSlopeLeft = [&](const auto... args) {
        if (left == TCT::SOLID) {
            const TCT belowLeft = getTc(x - 1, y + 1);
            if (belowLeft != TCT::SOLID) {
                ret |= InvalidRoomTile::INVALID_TILE_ON_THE_LEFT;
            }
        }
        else if ((... && (left != args))) {
            ret |= InvalidRoomTile::INVALID_TILE_ON_THE_LEFT;
        }
    };

    auto testDownSlopeRight = [&](const auto... args) {
        if (right == TCT::SOLID) {
            const TCT aboveRight = getTc(x + 1, y - 1);
            if (aboveRight != TCT::SOLID) {
                ret |= InvalidRoomTile::INVALID_TILE_ON_THE_RIGHT;
            }
        }
        else if ((... && (right != args))) {
            ret |= InvalidRoomTile::INVALID_TILE_ON_THE_RIGHT;
        }
    };

    auto testUpSlopeRight = [&](const auto... args) {
        if (right == TCT::SOLID) {
            const TCT belowRight = getTc(x + 1, y + 1);
            if (belowRight != TCT::SOLID) {
                ret |= InvalidRoomTile::INVALID_TILE_ON_THE_RIGHT;
            }
        }
        else if ((... && (right != args))) {
            ret |= InvalidRoomTile::INVALID_TILE_ON_THE_RIGHT;
        }
    };

    switch (tile) {
    case TCT::DOWN_RIGHT_SLOPE:
        testDownSlopeAboveBelow();
        testDownSlopeRight(TCT::DOWN_LEFT_SLOPE, TCT::DOWN_LEFT_TALL_SLOPE, TCT::END_SLOPE);
        break;

    case TCT::DOWN_LEFT_SLOPE:
        testDownSlopeAboveBelow();
        testDownSlopeLeft(TCT::DOWN_RIGHT_SLOPE, TCT::DOWN_RIGHT_TALL_SLOPE, TCT::END_SLOPE);
        break;

    case TCT::DOWN_RIGHT_SHORT_SLOPE:
        testDownSlopeAboveBelow();
        testDownSlopeRight(TCT::DOWN_RIGHT_TALL_SLOPE, TCT::DOWN_LEFT_SHORT_SLOPE);
        break;

    case TCT::DOWN_RIGHT_TALL_SLOPE:
        testDownSlopeAboveBelow();
        testDownSlopeLeft(TCT::DOWN_RIGHT_SHORT_SLOPE, TCT::DOWN_LEFT_TALL_SLOPE);
        testDownSlopeRight(TCT::DOWN_LEFT_SLOPE, TCT::DOWN_LEFT_TALL_SLOPE, TCT::END_SLOPE);
        break;

    case TCT::DOWN_LEFT_TALL_SLOPE:
        testDownSlopeAboveBelow();
        testDownSlopeLeft(TCT::DOWN_RIGHT_SLOPE, TCT::DOWN_RIGHT_TALL_SLOPE, TCT::END_SLOPE);
        testDownSlopeRight(TCT::DOWN_LEFT_SHORT_SLOPE, TCT::DOWN_RIGHT_TALL_SLOPE);
        break;

    case TCT::DOWN_LEFT_SHORT_SLOPE:
        testDownSlopeAboveBelow();
        testDownSlopeLeft(TCT::DOWN_LEFT_TALL_SLOPE, TCT::DOWN_RIGHT_SHORT_SLOPE);
        break;

    case TCT::UP_RIGHT_SLOPE:
        testUpSlopeAboveBelow();
        testUpSlopeRight(TCT::UP_LEFT_SLOPE, TCT::UP_LEFT_TALL_SLOPE, TCT::END_SLOPE);
        break;

    case TCT::UP_LEFT_SLOPE:
        testUpSlopeAboveBelow();
        testUpSlopeLeft(TCT::UP_RIGHT_SLOPE, TCT::UP_RIGHT_TALL_SLOPE, TCT::END_SLOPE);
        break;

    case TCT::UP_RIGHT_SHORT_SLOPE:
        testUpSlopeAboveBelow();
        testUpSlopeRight(TCT::UP_RIGHT_TALL_SLOPE, TCT::UP_LEFT_SHORT_SLOPE);
        break;

    case TCT::UP_RIGHT_TALL_SLOPE:
        testUpSlopeAboveBelow();
        testUpSlopeLeft(TCT::UP_RIGHT_SHORT_SLOPE, TCT::UP_LEFT_TALL_SLOPE);
        testUpSlopeRight(TCT::UP_LEFT_SLOPE, TCT::UP_LEFT_TALL_SLOPE, TCT::END_SLOPE);
        break;

    case TCT::UP_LEFT_TALL_SLOPE:
        testUpSlopeAboveBelow();
        testUpSlopeLeft(TCT::UP_RIGHT_SLOPE, TCT::UP_RIGHT_TALL_SLOPE, TCT::END_SLOPE);
        testUpSlopeRight(TCT::UP_LEFT_SHORT_SLOPE, TCT::UP_RIGHT_TALL_SLOPE);
        break;

    case TCT::UP_LEFT_SHORT_SLOPE:
        testUpSlopeAboveBelow();
        testUpSlopeLeft(TCT::UP_LEFT_TALL_SLOPE, TCT::UP_RIGHT_SHORT_SLOPE);
        break;

    case TCT::END_SLOPE:
        if (!isTileAllowedLeftofEndSlope(left)) {
            ret |= InvalidRoomTile::INVALID_TILE_ON_THE_LEFT;
        }
        if (!isTileAllowedRightofEndSlope(right)) {
            ret |= InvalidRoomTile::INVALID_TILE_ON_THE_RIGHT;
        }
        break;

    case TCT::EMPTY:
    case TCT::SOLID:
    case TCT::UP_PLATFORM:
    case TCT::DOWN_PLATFORM:
        break;
    }

    return ret;
}

static void checkRoomTileCollisions(const grid<uint8_t>& map,
                                    const MetaTiles::MetaTileTilesetData& tileset,
                                    ErrorList& err)
{
    assert(map.width() < INT_MAX);
    assert(map.height() < INT_MAX);

    if (map.width() < 3 || map.width() > RoomInput::MAX_MAP_WIDTH
        || map.height() < 3 || map.height() > RoomInput::MAX_MAP_HEIGHT) {

        err.addWarningString("Cannot check tile collisions: Invalid map size");
        return;
    }

    std::vector<InvalidRoomTile> invalidTiles;

    auto test = [&](auto checkTileCollisionFunction, unsigned x, unsigned y) {
        const uint8_t d = checkTileCollisionFunction(x, y, map, tileset);
        if (d != 0) {
            invalidTiles.emplace_back(x, y, d);
        }
    };

    // Top Row
    for (const auto x : range(map.width())) {
        test(checkTileCollision_Border, x, 0);
    }
    for (const auto y : range(1, map.height() - 1)) {
        test(checkTileCollision_Border, 0, y);

        for (const auto x : range(1, map.width() - 1)) {
            test(checkTileCollision_NotBorder, x, y);
        }

        test(checkTileCollision_Border, map.width() - 1, y);
    }
    for (const auto x : range(map.width())) {
        test(checkTileCollision_Border, x, map.height() - 1);
    }

    if (!invalidTiles.empty()) {
        err.addWarning(std::make_unique<InvalidRoomTilesError>(std::move(invalidTiles)));
    }
}

static bool validate(const RoomInput& input,
                     const Resources::CompiledScenesData& compiledScenes,
                     const Project::DataStore<MetaTiles::MetaTileTilesetData>& metaTilesData,
                     ErrorList& err)
{
    bool valid = true;
    auto addError = [&](const auto&... msg) {
        err.addErrorString(msg...);
        valid = false;
    };

    if (!input.name.isValid()) {
        addError("Missing name");
    }

    std::shared_ptr<const MetaTiles::MetaTileTilesetData> tileset;

    if (auto sceneData = compiledScenes.findScene(input.scene)) {
        if (sceneData->mtTileset) {
            tileset = metaTilesData.at(sceneData->mtTileset);
        }
        else {
            addError("Scene ", input.scene, " does not have a MetaTile layer");
        }
    }
    else {
        addError("Cannot find scene: ", input.scene);
    }

    if (input.map.width() < input.MIN_MAP_WIDTH || input.map.height() < input.MIN_MAP_HEIGHT) {
        addError("Map is too small (minimum size is ", input.MIN_MAP_WIDTH, " x ", input.MIN_MAP_HEIGHT, ")");
    }
    if (input.map.width() > input.MAX_MAP_WIDTH || input.map.height() > input.MAX_MAP_HEIGHT) {
        addError("Map is too large (maximum size is ", input.MAX_MAP_WIDTH, " x ", input.MAX_MAP_HEIGHT, ")");
    }

    valid &= validateEntrances(input.entrances, input, err);
    valid &= validateEntityGroups(input.entityGroups, input, err);
    valid &= validateScriptTriggers(input.scriptTriggers, input, err);

    if (tileset) {
        checkRoomTileCollisions(input.map, *tileset, err);
    }

    return valid;
}

static unsigned countEntities(const NamedList<EntityGroup>& entityGroups)
{
    unsigned count = 0;
    for (const auto& eg : entityGroups) {
        count += eg.entities.size();
    }

    return count;
}

static unsigned threeBytePosition(const unsigned x, const unsigned y)
{
    constexpr static unsigned POSITION_MASK = 0x0fff;
    constexpr static unsigned POSITION_SHIFT = 12;

    static_assert(1 << POSITION_SHIFT == POSITION_MASK + 1);
    static_assert(sizeof(int) > 3);

    static_assert(UINT8_MAX * MAP_TILE_SIZE < POSITION_MASK);                                      // X axis
    static_assert(MAP_HEIGHT_LARGE * MAP_TILE_SIZE + ENTITY_VERTICAL_SPACING * 2 < POSITION_MASK); // Y axis

    assert(x <= POSITION_MASK);
    assert(y <= POSITION_MASK);

    return (x & POSITION_MASK) | ((y & POSITION_MASK) << POSITION_SHIFT);
}

template <typename AddErrorFunction>
static uint8_t processEntityParameter(const std::string& parameter, const Entity::ParameterType parameterType,
                                      AddErrorFunction addEntityError)
{
    switch (parameterType) {
    case Entity::ParameterType::UNUSED: {
        if (!parameter.empty()) {
            addEntityError("Expected an empty parameter.");
        }
        return 0;
    }

    case Entity::ParameterType::UNSIGNED_BYTE: {
        const auto v = String::toUint8(parameter);
        if (!v.exists()) {
            addEntityError("Invalid parameter: ", parameter);
        }
        return v.value_or(0);
    }
    }

    return 0;
}

std::shared_ptr<const RoomData>
compileRoom(const RoomInput& input, const ExternalFileList<Rooms::RoomInput>& roomsList,
            const Resources::CompiledScenesData& compiledScenes, const Entity::CompiledEntityRomData& entityRomData, const RoomSettings& roomSettings,
            const Scripting::GameStateData& gameStateData, const Scripting::BytecodeMapping& bytecodeData,
            const Project::DataStore<MetaTiles::MetaTileTilesetData>& metaTilesData,
            ErrorList& err)
{
    bool valid = validate(input, compiledScenes, metaTilesData, err);

    const auto addError = [&](const auto... msg) {
        err.addErrorString(msg...);
        valid = false;
    };
    const auto addEntityGroupError = [&](const EntityGroup& eg, unsigned egIndex, const auto... msg) {
        err.addError(entityGroupError(eg, egIndex, msg...));
        valid = false;
    };
    const auto addEntityError = [&](const EntityGroup& eg, const unsigned egIndex, const unsigned eeIndex, const auto... msg) {
        err.addError(entityEntryError(eg, egIndex, eeIndex, msg...));
        valid = false;
    };

    const unsigned totalEntityCount = countEntities(input.entityGroups);

    // Create mappings
    // To be used by the scripting engine
    std::unordered_map<idstring, unsigned> entityGroupIndexMap;
    std::unordered_map<idstring, unsigned> entityNameIndexMap;
    {
        entityGroupIndexMap.reserve(input.entityGroups.size());

        for (auto [egIndex, eg] : const_enumerate(input.entityGroups)) {
            bool s = entityGroupIndexMap.emplace(eg.name, egIndex).second;
            if (!s) {
                addEntityGroupError(eg, egIndex, "Duplicate Entity Group id: ", eg.name);
            }
        }

        unsigned entityIndex = 0;
        for (auto [egIndex, eg] : const_enumerate(input.entityGroups)) {
            for (auto [eeIndex, ee] : const_enumerate(eg.entities)) {
                if (ee.name.isValid()) {
                    bool s = entityNameIndexMap.emplace(ee.name, entityIndex).second;
                    if (!s) {
                        addEntityError(eg, egIndex, eeIndex, "Duplicate Entity id: ", ee.name);
                    }
                }
                entityIndex++;
            }
        }
    }

    if (!valid) {
        return nullptr;
    }

    constexpr unsigned SCRIPT_HEADER_SIZE = (MAX_N_SCRIPTS + 1) * 2;
    constexpr unsigned SCRIPT_TRIGGERS_SIZE = MAX_SCRIPT_TRIGGERS * 6;
    constexpr unsigned HEADER_SIZE = 4 + SCRIPT_HEADER_SIZE + MAX_ENTITY_GROUPS + SCRIPT_TRIGGERS_SIZE;
    constexpr unsigned HEADER_SCRIPT_ARRAY = 4;

    const bool mapHeightBit = input.map.height() <= MAP_HEIGHT_SMALL;
    const unsigned mapHeight = mapHeightBit ? MAP_HEIGHT_SMALL : MAP_HEIGHT_LARGE;
    const unsigned mapDataSize = mapHeight * input.map.width();

    const unsigned roomEntranceDataSize = 1 + 4 * input.entrances.size();

    assert(totalEntityCount < MAX_ENTITY_ENTRIES);
    static_assert(MAX_ENTITY_ENTRIES <= UINT8_MAX / 2);
    const unsigned entityDataSize = 1 + totalEntityCount * 5;

    auto out = std::make_shared<RoomData>();

    std::vector<uint8_t>& data = out->data;
    data.reserve(roomSettings.roomDataSize);

    // Start of data with known size
    // -----------------------------

    data.resize(HEADER_SIZE + mapDataSize + roomEntranceDataSize + entityDataSize);
    auto it = data.begin();

    // Header
    {
        const unsigned sceneId = compiledScenes.indexForScene(input.scene).value_or(INT_MAX);
        assert(sceneId < UINT8_MAX);
        assert(input.entrances.size() < UINT8_MAX);

        *it++ = input.map.width();
        *it++ = input.map.height();
        *it++ = sceneId;
        *it++ = input.entrances.size();

        // Skip Header.startupScript and Header.scripts
        it += SCRIPT_HEADER_SIZE;

        // entityGroups array
        unsigned egPos = 0;
        for (const auto eg : range(MAX_ENTITY_GROUPS)) {
            if (eg < input.entityGroups.size()) {
                egPos += input.entityGroups.at(eg).entities.size();
                *it++ = egPos;
            }
            else {
                *it++ = 0;
            }
        }
        assert(egPos == totalEntityCount);

        // Script triggers
        {
            constexpr unsigned ONCE_FLAG = 0x80;
            constexpr unsigned soaSize = 2 * MAX_SCRIPT_TRIGGERS;

            assert(input.scriptTriggers.size() <= MAX_SCRIPT_TRIGGERS);

            auto buildArray = [&](auto f) {
                auto endIt = it + soaSize;

                for (auto& st : input.scriptTriggers) {
                    const uint16_t v = f(st);
                    *it++ = v;
                    *it++ = v >> 8;
                }
                assert(it <= endIt);

                while (it < endIt) {
                    *it++ = 0xff;
                }
            };

            buildArray([&](auto& st) { return input.tileIndex(st.aabb.topLeft()); });
            buildArray([&](auto& st) { return input.tileIndex(st.aabb.internalBottomRight()) + 1; });

            buildArray([&](auto& st) -> uint16_t {
                auto scriptId = input.roomScripts.scripts.indexOf(st.script);
                assert(scriptId < input.roomScripts.scripts.size());

                if (st.once) {
                    scriptId |= ONCE_FLAG;
                }
                return scriptId;
            });
        }

        assert(it == data.begin() + HEADER_SIZE);
    }

    // Map data
    {
        const auto begin = it;

        for (const auto x : range(input.map.width())) {
            for (const auto y : range(input.map.height())) {
                *it++ = input.map.at(x, y);
            }
            if (input.map.height() < mapHeight) {
                it += mapHeight - input.map.height();
            }
        }
        assert(it == begin + mapDataSize);
    }

    // ::TODO add submaps::

    // Room Entrance Data
    {
        assert(input.entrances.size() > 0);
        assert(input.entrances.size() < MAX_ROOM_ENTRANCES);

        const auto begin = it;

        *it++ = 'N';

        for (const RoomEntrance& en : input.entrances) {
            const unsigned pos = threeBytePosition(en.position.x, en.position.y);

            *it++ = (pos >> 0) & 0xff;
            *it++ = (pos >> 8) & 0xff;
            *it++ = (pos >> 16) & 0xff;
            *it++ = uint8_t(en.orientation);
        }

        assert(it == begin + roomEntranceDataSize);
    }

    // RoomEntity Data
    {
        assert(input.entityGroups.size() < MAX_ENTITY_GROUPS);

        const auto begin = it;

        *it++ = 'E';

        for (auto egIt : enumerate(input.entityGroups)) {
            const auto egIndex = egIt.first;
            const auto& eg = egIt.second;

            assert(eg.entities.size() > 0);

            for (auto eeIt : const_enumerate(eg.entities)) {
                const auto eeIndex = eeIt.first;
                const auto& ee = eeIt.second;

                static_assert(ENTITY_VERTICAL_SPACING == 256);

                assert(ee.position.x >= 0);
                assert(ee.position.y >= -ENTITY_VERTICAL_SPACING);

                const unsigned pos = threeBytePosition(ee.position.x, ee.position.y + ENTITY_VERTICAL_SPACING);

                unsigned entityId = 0;
                uint8_t parameter = 0;

                const auto enIt = entityRomData.entityNameMap.find(ee.entityId);
                if (enIt != entityRomData.entityNameMap.end()) {
                    entityId = enIt->second.first;

                    const auto parameterType = enIt->second.second;
                    parameter = processEntityParameter(ee.parameter, parameterType,
                                                       [&](const auto&... msg) { addEntityError(eg, egIndex, eeIndex, msg...); });
                }
                else {
                    addEntityError(eg, egIndex, eeIndex, "Cannot find entity ", ee.entityId);
                }
                assert(entityId <= UINT8_MAX);

                *it++ = entityId;
                *it++ = (pos >> 0) & 0xff;
                *it++ = (pos >> 8) & 0xff;
                *it++ = (pos >> 16) & 0xff;
                *it++ = parameter;
            }
        }

        assert(it == begin + entityDataSize);
    }
    assert(it == data.end());

    // End of data with known size

    // Start of data with unknown size
    // --------------------------------

    // MUST NOT USE it beyond this point.

    // Room Scripts
    {
        data.push_back('S');

        // size of script block (unknown)
        const unsigned blockSizePos = data.size();
        data.push_back(0);
        data.push_back(0);

        Scripting::ScriptCompiler compiler(data, input, roomsList, bytecodeData, gameStateData, err);

        if (input.roomScripts.scripts.size() > MAX_N_SCRIPTS) {
            addError("Too many scripts");
        }

        unsigned headerPos = HEADER_SCRIPT_ARRAY;
        constexpr unsigned headerPosEnd = HEADER_SCRIPT_ARRAY + SCRIPT_HEADER_SIZE;

        // Startup Script
        {
            const unsigned scriptPos = compiler.compileScript(input.roomScripts.startupScript, INT_MAX);

            data.at(headerPos++) = scriptPos & 0xff;
            data.at(headerPos++) = (scriptPos >> 8) & 0xff;
        }

        for (auto [i, s] : enumerate(input.roomScripts.scripts)) {
            const unsigned scriptPos = compiler.compileScript(s, i);

            // Populate scriptPos in the room header
            if (headerPos < headerPosEnd) {
                data.at(headerPos++) = scriptPos & 0xff;
                data.at(headerPos++) = (scriptPos >> 8) & 0xff;
            }
        }

        // Set unused script indexes to NULL
        while (headerPos < headerPosEnd) {
            data.at(headerPos++) = 0;
            data.at(headerPos++) = 0;
        }
        assert(headerPos == headerPosEnd);

        // Store size of script block
        const unsigned blockSize = data.size() - blockSizePos - 2;
        data.at(blockSizePos) = blockSize & 0xff;
        data.at(blockSizePos + 1) = (blockSize >> 8) & 0xff;

        valid &= compiler.isValid();
    }

    // End marker
    {
        constexpr static std::array<uint8_t, 4> endMarker = { 'E', 'N', 'D', '!' };

        data.insert(data.end(), endMarker.begin(), endMarker.end());
    }

    if (data.size() > roomSettings.roomDataSize) {
        addError("Room data size too large (", data.size(), " bytes, max: ", roomSettings.roomDataSize, ")");
    }

    if (!valid) {
        out = nullptr;
    }
    else if (data.empty()) {
        out = nullptr;
    }

    return out;
}

const int RoomData::ROOM_FORMAT_VERSION = 6;

std::vector<uint8_t> RoomData::exportSnesData() const
{
    return lz4HcCompress(data);
}

}
