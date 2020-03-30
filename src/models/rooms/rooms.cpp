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

    return valid;
}

}
