/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

namespace UnTech {
namespace GuiQt {

enum class ResourceState {
    NOT_LOADED,
    UNCHECKED,
    VALID,
    ERROR,
    FILE_ERROR,
    DEPENDENCY_ERROR,
};

enum class ResourceTypeIndex {
    STATIC,
    PALETTE,
    MT_TILESET,
    MS_FRAMESET,
    MS_EXPORT_ORDER,
    BACKGROUND_IMAGE,
    ROOM,
};

constexpr unsigned N_RESOURCE_TYPES = 7;
}
}
