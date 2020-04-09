/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "rooms.h"
#include "models/common/errorlist.h"
#include "models/project/project-data.h"
#include "models/resources/scenes.h"

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

static bool validateEntityGroups(const NamedList<EntityGroup>& entityGroups, const usize& mapSize, ErrorList& err)
{
    bool valid = true;

    // ::TODO validate EntityGroup/EntityEntry names (in converter)::

    const auto addError = [&](const auto... msg) {
        err.addErrorString(msg...);
        valid = false;
    };
    const auto addEntityError = [&](const EntityEntry& ee, const auto... msg) {
        err.addError(std::make_unique<ListItemError>(&ee, msg...));
        valid = false;
    };

    if (entityGroups.size() > MAX_ENTITY_GROUPS) {
        addError("Too many Entity Groups (", entityGroups.size(), ", max: ", MAX_ENTITY_GROUPS, ")");
    }

    const rect mapBounds{
        0, -ENTITY_VERTICAL_SPACING,
        mapSize.width * MAP_TILE_SIZE, mapSize.height + MAP_TILE_SIZE + ENTITY_VERTICAL_SPACING * 2
    };

    unsigned count = 0;
    for (auto& eg : entityGroups) {
        for (const EntityEntry& ee : eg.entities) {
            if (mapBounds.contains(ee.position) == false) {
                addEntityError(ee, "Entity outsize of map bounds");
            }
            count++;
        }
    }

    if (count > MAX_ENTITY_ENTRIES) {
        addError("Too many Entities (", count, ", max: ", MAX_ENTITY_ENTRIES, ")");
    }

    return valid;
}

bool RoomInput::validate(const Project::ProjectData& projectData, ErrorList& err) const
{
    bool valid = true;
    auto addError = [&](const auto&... msg) {
        err.addErrorString(msg...);
        valid = false;
    };

    if (!name.isValid()) {
        addError("Missing name");
    }

    if (auto sceneData = projectData.scenes()->findScene(scene)) {
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

    valid &= validateEntityGroups(entityGroups, map.size(), err);

    return valid;
}

}
