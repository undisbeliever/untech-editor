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
#include "models/common/mutex_wrapper.h"
#include <unordered_map>
#include <vector>

namespace UnTech::Project {
struct ProjectFile;
struct ProjectData;
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

    explicit EntityGraphics(const usize& imageSize);

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
    struct State {
        std::shared_ptr<const EntityGraphics> data{};
        unsigned entityRomDataCompileId = 0;
    };
    mutex<State> _state;

public:
    EntityGraphicsStore();

    std::shared_ptr<const EntityGraphics> get()
    {
        return _state.access_and_return_const_shared_ptr<EntityGraphics>([&](auto& s) {
            return s.data;
        });
    }

    unsigned getEntityRomDataCompileId()
    {
        return _state.access_and_return_unsigned([&](auto& s) {
            return s.entityRomDataCompileId;
        });
    }

    void set(std::shared_ptr<const EntityGraphics> d, unsigned erdCompileId)
    {
        _state.access([&](auto& s) {
            s.data = std::move(d);
            s.entityRomDataCompileId = erdCompileId;
        });
    }
};

extern EntityGraphicsStore entityGraphicsStore;

void processEntityGraphics(const Project::ProjectFile& projectFile,
                           const Project::ProjectData& projectData,
                           const uint64_t entityRomDataCompileId);
}
