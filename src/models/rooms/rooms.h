/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/grid.h"
#include "models/common/idstring.h"

namespace UnTech {
class ErrorList;
namespace Project {
class ProjectData;
}

namespace Rooms {

struct RoomInput {
    constexpr static unsigned MIN_MAP_WIDTH = 16;
    constexpr static unsigned MIN_MAP_HEIGHT = 14;

    constexpr static unsigned MAX_MAP_WIDTH = 255;
    constexpr static unsigned MAX_MAP_HEIGHT = 128;

    static const std::string FILE_EXTENSION;

    idstring name;

    idstring scene;

    grid<uint8_t> map;

    bool validate(const Project::ProjectData& projectData, ErrorList& err) const;

    bool operator==(const RoomInput& o) const
    {
        return name == o.name
               && scene == o.scene
               && map == o.map;
    }
    bool operator!=(const RoomInput& o) const { return !(*this == o); }
};

}
}
