/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/errorlist.h"
#include "models/common/idstring.h"
#include "models/common/optional.h"
#include <memory>
#include <unordered_map>
#include <vector>

namespace UnTech::MetaSprite {
using ActionPointMapping = std::unordered_map<idstring, uint8_t>;
}
namespace UnTech::MetaSprite::Compiler {
struct FrameSetData;
}

namespace UnTech {
class ErrorList;

namespace Resources {
struct PaletteData;
struct BackgroundImageData;
struct SceneSettingsData;
struct CompiledScenesData;
}
namespace MetaTiles {
struct MetaTileTilesetData;
struct InteractiveTilesData;
}
namespace Entity {
struct CompiledEntityRomData;
}
namespace Rooms {
struct RoomData;
}

namespace GuiQt {
class ProjectDataSlots;
}
}

namespace UnTech {
namespace Project {
struct ProjectFile;

enum class ResourceState {
    Unchecked,
    Valid,
    Invalid,
    Missing,
};

struct ResourceStatus {
    ResourceState state = ResourceState::Unchecked;
    std::string name;
    ErrorList errorList;
};

struct ResourceListStatus {
    const std::string typeNameSingle;
    const std::string typeNamePlural;

    ResourceState state;
    std::vector<ResourceStatus> resources;

    ResourceListStatus(std::string typeNameSingle, std::string typeNamePlural);

    void clearAllAndResize(size_t size);
    void updateState();
};

template <typename T>
class DataStore {
    ResourceListStatus _listStatus;
    std::unordered_map<idstring, size_t> _mapping;
    std::vector<std::unique_ptr<const T>> _data;

public:
    DataStore(std::string typeNameSingle, std::string typeNamePlural);

    const auto& listStatus() const { return _listStatus; }

    inline std::optional<unsigned> indexOf(const idstring& id) const
    {
        auto it = _mapping.find(id);
        if (it != _mapping.end()) {
            return it->second;
        }
        else {
            return std::nullopt;
        }
    }

    inline optional<const T&> at(unsigned index) const
    {
        if (index < _data.size()) {
            return _data.at(index);
        }
        else {
            return {};
        }
    }
    inline optional<const T&> at(std::optional<unsigned> index) const
    {
        if (index) {
            return at(index.value());
        }
        else {
            return {};
        }
    }
    inline optional<const T&> at(const idstring& id) const
    {
        auto it = _mapping.find(id);
        if (it != _mapping.end()) {
            return at(unsigned(it->second));
        }
        else {
            return {};
        }
    }

    size_t size() const { return _data.size(); }

    void store(const size_t index, ResourceStatus&& status, std::unique_ptr<const T>&& data);

    void updateState() { _listStatus.updateState(); }

    void clearAllAndResize(size_t size)
    {
        _listStatus.clearAllAndResize(size);
        _mapping.clear();
        _mapping.reserve(size);
        _data.clear();
        _data.resize(size);
    }
};

class ProjectData {
private:
    enum class ProjectSettingsIndex : unsigned {
        MemoryMap,
        RoomSettings,
        InteractiveTiles,
        ActionPoints,
        EntityRomData,
        SceneSettings,
        Scenes,
    };
    constexpr static unsigned N_PROJECT_SETTING_ITEMS = 7;

    const ProjectFile& _project;

    ResourceListStatus _projectSettingsStatus;
    ResourceListStatus _frameSetExportOrderStatus;

    DataStore<UnTech::MetaSprite::Compiler::FrameSetData> _frameSets;
    DataStore<Resources::PaletteData> _palettes;
    DataStore<Resources::BackgroundImageData> _backgroundImages;
    DataStore<MetaTiles::MetaTileTilesetData> _metaTileTilesets;
    DataStore<Rooms::RoomData> _rooms;

    std::optional<MetaSprite::ActionPointMapping> _actionPointMapping;
    std::unique_ptr<const MetaTiles::InteractiveTilesData> _interactiveTiles;
    std::unique_ptr<const Resources::SceneSettingsData> _sceneSettings;
    std::unique_ptr<const Resources::CompiledScenesData> _scenes;
    std::unique_ptr<const Entity::CompiledEntityRomData> _entityRomData;

private:
    ProjectData(const ProjectData&) = delete;
    ProjectData(ProjectData&&) = delete;
    ProjectData& operator=(const ProjectData&) = delete;
    ProjectData& operator=(ProjectData&&) = delete;

public:
    ProjectData(const ProjectFile& project);

    const auto& projectSettingsStatus() const { return _projectSettingsStatus; }
    const auto& frameSetExportOrderStatus() const { return _frameSetExportOrderStatus; }

    const DataStore<UnTech::MetaSprite::Compiler::FrameSetData>& frameSets() const { return _frameSets; }
    const DataStore<Resources::PaletteData>& palettes() const { return _palettes; }
    const DataStore<Resources::BackgroundImageData>& backgroundImages() const { return _backgroundImages; }
    const DataStore<MetaTiles::MetaTileTilesetData>& metaTileTilesets() const { return _metaTileTilesets; }
    const DataStore<Rooms::RoomData>& rooms() const { return _rooms; }

    const optional<const MetaTiles::InteractiveTilesData&> interactiveTiles() const { return _interactiveTiles; }
    const optional<const Resources::SceneSettingsData&> sceneSettings() const { return _sceneSettings; }
    const optional<const Resources::CompiledScenesData&> scenes() const { return _scenes; }
    const optional<const Entity::CompiledEntityRomData&> entityRomData() const { return _entityRomData; }

    bool compileAll();

private:
    bool storePsStatus(ProjectSettingsIndex index, ResourceStatus&& newStatus);
};

}
}
