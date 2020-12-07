/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

namespace UnTech {

enum class ResourceType : unsigned {
    ProjectSettings,
    FrameSetExportOrders,
    FrameSets,
    Palettes,
    BackgroundImages,
    MataTileTilesets,
    Rooms,
};
constexpr unsigned N_RESOURCE_TYPES = 7;

enum class ProjectSettingsIndex : unsigned {
    ProjectSettings,
    GameState,
    Bytecode,
    InteractiveTiles,
    ActionPoints,
    EntityRomData,
    Scenes,
};
constexpr unsigned N_PROJECT_SETTING_ITEMS = 7;

}
