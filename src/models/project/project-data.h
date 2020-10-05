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
#include <shared_mutex>
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

class ResourceListStatus {
protected:
    mutable std::shared_mutex _mutex;

    const std::string _typeNameSingle;
    const std::string _typeNamePlural;

    ResourceState _state;
    std::vector<ResourceStatus> _resources;

private:
    ResourceListStatus(const ResourceListStatus&) = delete;
    ResourceListStatus(ResourceListStatus&&) = delete;
    ResourceListStatus& operator=(const ResourceListStatus&) = delete;
    ResourceListStatus& operator=(ResourceListStatus&&) = delete;

public:
    ResourceListStatus(std::string typeNameSingle, std::string typeNamePlural);

    const std::string& typeNameSingle() const { return _typeNameSingle; }
    const std::string& typeNamePlural() const { return _typeNamePlural; }

    // Must include a lock in each function
    // MUST NOT add a write functions in the header file

    void clearAllAndResize(size_t size);
    void setStatus(unsigned index, ResourceStatus&& status);
    void setStatusKeepName(unsigned index, ResourceStatus&& status);
    void updateState();

    template <typename ListT>
    void clearAllAndPopulateNames(const ListT& list);

    template <typename Function>
    void readResourceListState(Function f) const
    {
        std::shared_lock lock(_mutex);

        f(const_cast<const ResourceState&>(_state),
          const_cast<const std::vector<ResourceStatus>&>(_resources));
    }
};

template <typename T>
class DataStore final : public ResourceListStatus {
    std::unordered_map<idstring, size_t> _mapping;
    std::vector<std::shared_ptr<const T>> _data;

public:
    DataStore(std::string typeNameSingle, std::string typeNamePlural);

    // MUST include a lock in each function
    // MUST NOT implement a write functions in the header file

    inline std::optional<unsigned> indexOf(const idstring& id) const
    {
        std::shared_lock lock(_mutex);

        auto it = _mapping.find(id);
        if (it != _mapping.end()) {
            return it->second;
        }
        else {
            return std::nullopt;
        }
    }

    inline std::shared_ptr<const T> at(unsigned index) const
    {
        std::shared_lock lock(_mutex);

        if (index < _data.size()) {
            return _data.at(index);
        }
        else {
            return {};
        }
    }

    inline std::shared_ptr<const T> at(std::optional<unsigned> index) const
    {
        std::shared_lock lock(_mutex);

        if (index) {
            return at(index.value());
        }
        else {
            return {};
        }
    }

    inline std::shared_ptr<const T> at(const idstring& id) const
    {
        std::shared_lock lock(_mutex);

        auto it = _mapping.find(id);
        if (it != _mapping.end()) {
            return at(it->second);
        }
        else {
            return {};
        }
    }

    size_t size() const
    {
        std::shared_lock lock(_mutex);

        return _data.size();
    }

    void clearAllAndResize(size_t size);
    void store(const size_t index, ResourceStatus&& status, std::shared_ptr<const T>&& data);
};

class ProjectSettingsData {
private:
    mutable std::shared_mutex _mutex;

    std::shared_ptr<const MetaSprite::ActionPointMapping> _actionPointMapping;
    std::shared_ptr<const MetaTiles::InteractiveTilesData> _interactiveTiles;
    std::shared_ptr<const Resources::CompiledScenesData> _scenes;
    std::shared_ptr<const Entity::CompiledEntityRomData> _entityRomData;

public:
    // MUST include a lock in each function
    // MUST NOT implement a write functions in the header file

    std::shared_ptr<const MetaSprite::ActionPointMapping> actionPointMapping() const
    {
        std::shared_lock lock(_mutex);
        return _actionPointMapping;
    }

    std::shared_ptr<const MetaTiles::InteractiveTilesData> interactiveTiles() const
    {
        std::shared_lock lock(_mutex);
        return _interactiveTiles;
    }

    std::shared_ptr<const Resources::CompiledScenesData> scenes() const
    {
        std::shared_lock lock(_mutex);
        return _scenes;
    }

    std::shared_ptr<const Entity::CompiledEntityRomData> entityRomData() const
    {
        std::shared_lock lock(_mutex);
        return _entityRomData;
    }

    void store(std::shared_ptr<const MetaSprite::ActionPointMapping>&& data);
    void store(std::shared_ptr<const MetaTiles::InteractiveTilesData>&& data);
    void store(std::shared_ptr<const Resources::CompiledScenesData>&& data);
    void store(std::shared_ptr<const Entity::CompiledEntityRomData>&& data);
};

class ProjectData {
private:
    enum class ProjectSettingsIndex : unsigned {
        ProjectSettings,
        InteractiveTiles,
        ActionPoints,
        EntityRomData,
        Scenes,
    };
    constexpr static unsigned N_PROJECT_SETTING_ITEMS = 6;

    ResourceListStatus _projectSettingsStatus;
    ResourceListStatus _frameSetExportOrderStatus;

    ProjectSettingsData _projectSettingsData;

    DataStore<UnTech::MetaSprite::Compiler::FrameSetData> _frameSets;
    DataStore<Resources::PaletteData> _palettes;
    DataStore<Resources::BackgroundImageData> _backgroundImages;
    DataStore<MetaTiles::MetaTileTilesetData> _metaTileTilesets;
    DataStore<Rooms::RoomData> _rooms;

private:
    ProjectData(const ProjectData&) = delete;
    ProjectData(ProjectData&&) = delete;
    ProjectData& operator=(const ProjectData&) = delete;
    ProjectData& operator=(ProjectData&&) = delete;

public:
    ProjectData();

    const auto& projectSettingsStatus() const { return _projectSettingsStatus; }
    const auto& frameSetExportOrderStatus() const { return _frameSetExportOrderStatus; }

    const DataStore<UnTech::MetaSprite::Compiler::FrameSetData>& frameSets() const { return _frameSets; }
    const DataStore<Resources::PaletteData>& palettes() const { return _palettes; }
    const DataStore<Resources::BackgroundImageData>& backgroundImages() const { return _backgroundImages; }
    const DataStore<MetaTiles::MetaTileTilesetData>& metaTileTilesets() const { return _metaTileTilesets; }
    const DataStore<Rooms::RoomData>& rooms() const { return _rooms; }

    std::shared_ptr<const MetaTiles::InteractiveTilesData> interactiveTiles() const { return _projectSettingsData.interactiveTiles(); }
    std::shared_ptr<const Resources::CompiledScenesData> scenes() const { return _projectSettingsData.scenes(); }
    std::shared_ptr<const Entity::CompiledEntityRomData> entityRomData() const { return _projectSettingsData.entityRomData(); }

    bool compileAll_EarlyExit(const ProjectFile& project) { return compileAll(project, true); }
    bool compileAll_NoEarlyExit(const ProjectFile& project) { return compileAll(project, false); }

private:
    bool compileAll(const ProjectFile& project, const bool earlyExit);

    bool storePsStatus(ProjectSettingsIndex index, ResourceStatus&& newStatus);
};

}
}
