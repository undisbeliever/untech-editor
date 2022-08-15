/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "two-point-rect.h"
#include "gui/imgui-drawing.h"
#include "models/common/idstring.h"
#include "models/common/image.h"
#include <mutex>
#include <unordered_map>
#include <vector>

namespace UnTech::Project {
struct ProjectFile;
class ProjectData;
}

namespace UnTech::Gui {

struct DrawEntitySettings {
    idstring name;
    TwoPointRect hitboxRect;
    TwoPointRect imageRect;
    ImVec2 uvMin;
    ImVec2 uvMax;
};

struct EntityGraphics {
    Image image;
    DrawEntitySettings nullSetting;
    std::unordered_map<idstring, unsigned> entityNameMap;
    std::vector<DrawEntitySettings> entities;
    std::vector<DrawEntitySettings> players;

    EntityGraphics(const usize& imageSize);

    const DrawEntitySettings& settingsForPlayer(unsigned playerId) const
    {
        if (playerId < players.size()) {
            return players.at(playerId);
        }
        return nullSetting;
    }

    const DrawEntitySettings& settingsForEntity(const idstring& name) const
    {
        auto it = entityNameMap.find(name);
        if (it != entityNameMap.end()) {
            return entities.at(it->second);
        }
        return nullSetting;
    }
};

class EntityGraphicsStore {
private:
    std::mutex _mutex;
    std::shared_ptr<const EntityGraphics> _data;
    unsigned _entityRomDataCompileId;

public:
    EntityGraphicsStore();

    std::shared_ptr<const EntityGraphics> get()
    {
        std::lock_guard lock(_mutex);

        return _data;
    }

    void set(std::shared_ptr<const EntityGraphics> d, unsigned erdCompileId)
    {
        std::lock_guard lock(_mutex);

        _data = d;
        _entityRomDataCompileId = erdCompileId;
    }

    unsigned getEntityRomDataCompileId()
    {
        std::lock_guard lock(_mutex);

        return _entityRomDataCompileId;
    }
};
extern EntityGraphicsStore entityGraphicsStore;

void processEntityGraphics(const Project::ProjectFile& projectFile,
                           const Project::ProjectData& projectData);
}
