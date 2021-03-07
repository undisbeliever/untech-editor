/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "rooms.h"
#include "models/common/errorlist.h"
#include "models/common/string.h"
#include "models/common/validateunique.h"
#include "models/entity/entityromdata.h"
#include "models/lz4/lz4.h"
#include "models/metatiles/common.h"
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

bool RoomSettings::validate(ErrorList& err) const
{
    bool valid = true;

    auto validateMinMax = [&](unsigned value, unsigned min, unsigned max, const char* msg) {
        if (value < min || value > max) {
            err.addErrorString(msg, " (", value, ", min: ", min, ", max: ", max, ")");
            valid = false;
        }
    };

    validateMinMax(roomDataSize, MIN_ROOM_DATA_SIZE, MAX_ROOM_DATA_SIZE, "Max Room Size invalid");

    return valid;
}

static bool validateEntrances(const NamedList<RoomEntrance>& entrances, const RoomInput& ri, ErrorList& err)
{
    bool valid = true;

    const auto addError = [&](const auto... msg) {
        err.addErrorString(msg...);
        valid = false;
    };
    const auto addEntranceError = [&](const RoomEntrance& e, const auto... msg) {
        err.addError(std::make_unique<ListItemError>(&e, "Entity Entrance ", e.name, ": ", msg...));
        valid = false;
    };

    if (entrances.empty()) {
        addError("Expected at least one Room Entrance");
    }

    if (entrances.size() > MAX_ROOM_ENTRANCES) {
        addError("Too many Room Entrances (", entrances.size(), ", max: ", MAX_ROOM_ENTRANCES, ")");
    }

    valid &= validateNamesUnique(entrances, "Room Entrance", err);

    for (auto& en : entrances) {
        if (en.position.x >= ri.mapRight() || en.position.y >= ri.mapBottom()) {
            addEntranceError(en, "Entrance must be inside map");
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
    const auto addEntityGroupError = [&](const EntityGroup& ee, const auto... msg) {
        err.addError(std::make_unique<ListItemError>(&ee, "Entity Group ", ee.name, ":", msg...));
        valid = false;
    };
    const auto addEntityError = [&](const EntityEntry& ee, const auto... msg) {
        err.addError(std::make_unique<ListItemError>(&ee, msg...));
        valid = false;
    };

    if (entityGroups.size() > MAX_ENTITY_GROUPS) {
        addError("Too many Entity Groups (", entityGroups.size(), ", max: ", MAX_ENTITY_GROUPS, ")");
    }

    const rect mapBounds = ri.validEntityArea();

    unsigned count = 0;
    for (auto& eg : entityGroups) {
        if (eg.name.isValid() == false) {
            addEntityGroupError(eg, "Expected name");
        }

        if (eg.entities.empty()) {
            addEntityGroupError(eg, "Expected at least one entity");
        }

        for (const EntityEntry& ee : eg.entities) {
            if (mapBounds.contains(ee.position) == false) {
                addEntityError(ee, "Entity outside of map bounds");
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

    if (scriptTriggers.size() > MAX_SCRIPT_TRIGGERS) {
        addError("Too many Entity Groups (", scriptTriggers.size(), ", max: ", MAX_SCRIPT_TRIGGERS, ")");
    }

    for (unsigned i = 0; i < scriptTriggers.size(); i++) {
        const auto& st = scriptTriggers.at(i);

        const auto addStError = [&](const ScriptTrigger& st, const auto... msg) {
            err.addError(std::make_unique<ListItemError>(&st, "Script Trigger ", i, ": ", msg...));
            valid = false;
        };

        if (st.script.isValid()) {
            if (!room.roomScripts.scripts.find(st.script)) {
                addStError(st, "Cannot find script: ", st.script);
            }
        }
        else {
            addStError(st, "Expected script");
        }

        if (st.aabb.width <= 0 || st.aabb.height <= 0) {
            addStError(st, "Invalid AABB");
        }
        if (!room.map.size().contains(st.aabb)) {
            addStError(st, "AABB must be inside the room");
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

bool RoomInput::validate(const Resources::CompiledScenesData& compiledScenes, ErrorList& err) const
{
    bool valid = true;
    auto addError = [&](const auto&... msg) {
        err.addErrorString(msg...);
        valid = false;
    };

    if (!name.isValid()) {
        addError("Missing name");
    }

    if (auto sceneData = compiledScenes.findScene(scene)) {
        if (sceneData->mtTileset == std::nullopt) {
            addError("Scene ", scene, " does not have a MetaTile layer");
        }
    }
    else {
        addError("Cannot find scene: ", scene);
    }

    if (map.width() < MIN_MAP_WIDTH || map.height() < MIN_MAP_HEIGHT) {
        addError("Map is too small (minimum size is ", MIN_MAP_WIDTH, " x ", MIN_MAP_HEIGHT, ")");
    }
    if (map.width() > MAX_MAP_WIDTH || map.height() > MAX_MAP_HEIGHT) {
        addError("Map is too large (maximum size is ", MAX_MAP_WIDTH, " x ", MAX_MAP_HEIGHT, ")");
    }

    valid &= validateEntrances(entrances, *this, err);
    valid &= validateEntityGroups(entityGroups, *this, err);
    valid &= validateScriptTriggers(scriptTriggers, *this, err);

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
            const Resources::CompiledScenesData& compiledScenes, const Entity::CompiledEntityRomData& entityRomData, const RoomSettings& roomSettings, const Scripting::GameStateData& gameStateData,
            const Scripting::BytecodeMapping& bytecodeData,
            ErrorList& err)
{
    bool valid = input.validate(compiledScenes, err);

    const auto addError = [&](const auto... msg) {
        err.addErrorString(msg...);
        valid = false;
    };
    const auto addEntityGroupError = [&](const EntityGroup& eg, const auto... msg) {
        err.addError(std::make_unique<ListItemError>(&eg, msg...));
        valid = false;
    };
    const auto addEntityError = [&](const EntityEntry& ee, const auto... msg) {
        err.addError(std::make_unique<ListItemError>(&ee, msg...));
        valid = false;
    };

    const unsigned totalEntityCount = countEntities(input.entityGroups);

    // Create mappings
    // To be used by the scripting engine
    std::unordered_map<idstring, unsigned> entityGroupIndexMap;
    std::unordered_map<idstring, unsigned> entityNameIndexMap;
    {
        entityGroupIndexMap.reserve(input.entityGroups.size());

        for (unsigned egIndex = 0; egIndex < input.entityGroups.size(); egIndex++) {
            auto& eg = input.entityGroups.at(egIndex);

            bool s = entityGroupIndexMap.emplace(eg.name, egIndex).second;
            if (!s) {
                addEntityGroupError(eg, "Duplicate Entity Group id: ", eg.name);
            }
        }

        unsigned entityIndex = 0;
        for (auto& eg : input.entityGroups) {
            for (auto& ee : eg.entities) {
                if (ee.name.isValid()) {
                    bool s = entityNameIndexMap.emplace(ee.name, entityIndex).second;
                    if (!s) {
                        addEntityError(ee, "Duplicate Entity id: ", ee.name);
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
        for (unsigned eg = 0; eg < MAX_ENTITY_GROUPS; eg++) {
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

        for (unsigned x = 0; x < input.map.width(); x++) {
            for (unsigned y = 0; y < input.map.height(); y++) {
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

        for (const EntityGroup& eg : input.entityGroups) {
            assert(eg.entities.size() > 0);

            for (const EntityEntry& ee : eg.entities) {
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
                                                       [&](const auto&... msg) { addEntityError(ee, msg...); });
                }
                else {
                    addEntityError(ee, "Cannot find entity ", ee.entityId);
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
            const unsigned scriptPos = compiler.compileScript(input.roomScripts.startupScript);

            data.at(headerPos++) = scriptPos & 0xff;
            data.at(headerPos++) = (scriptPos >> 8) & 0xff;
        }

        for (const auto& s : input.roomScripts.scripts) {
            const unsigned scriptPos = compiler.compileScript(s);

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

    return out;
}

const int RoomData::ROOM_FORMAT_VERSION = 6;

std::vector<uint8_t> RoomData::exportSnesData() const
{
    return lz4HcCompress(data);
}

}
