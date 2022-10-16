/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/errorlist.h"
#include "models/common/idstring.h"
#include "models/common/optional_ref.h"
#include "models/enums.h"
#include <array>
#include <memory>
#include <optional>
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

template <typename T>
class ExternalFileList;
}

namespace UnTech::Scripting {
struct GameStateData;
struct BytecodeMapping;
}

namespace UnTech::Resources {
struct PaletteData;
struct BackgroundImageData;
struct SceneSettingsData;
struct CompiledScenesData;
}

namespace UnTech::MetaTiles {
struct MetaTileTilesetData;
struct InteractiveTilesData;
}

namespace UnTech::Entity {
struct CompiledEntityRomData;
}

namespace UnTech::Rooms {
struct RoomData;
}

namespace UnTech::Project {

struct ProjectFile;
class ProjectData;

enum class ResourceState {
    Unchecked,
    Valid,
    Invalid,
    Missing,
};

struct ResourceStatus {
    ResourceState state = ResourceState::Unchecked;
    std::u8string name;
    ErrorList errorList;

    // Increases every time the resource has been compiled.
    // 0 if the resource state is UnChecked.
    unsigned compileId = 0;
};

class ResourceListStatus {
protected:
    mutable std::shared_mutex _mutex;

    const std::u8string _typeNameSingle;
    const std::u8string _typeNamePlural;

    unsigned _currentCompileId;

    ResourceState _state;
    std::vector<ResourceStatus> _resources;

public:
    ResourceListStatus(const ResourceListStatus&) = delete;
    ResourceListStatus(ResourceListStatus&&) = delete;
    ResourceListStatus& operator=(const ResourceListStatus&) = delete;
    ResourceListStatus& operator=(ResourceListStatus&&) = delete;

    ~ResourceListStatus() = default;

public:
    ResourceListStatus(std::u8string typeNameSingle, std::u8string typeNamePlural);

    const std::u8string& typeNameSingle() const { return _typeNameSingle; }
    const std::u8string& typeNamePlural() const { return _typeNamePlural; }

    // Must include a lock in each function
    // MUST NOT add a write functions in the header file

    void clearAllAndResize(size_t size);

    // Returns the old name and current name of the resource
    std::pair<std::u8string, std::u8string> setStatus(unsigned index, ResourceStatus&& status);

    void setStatusKeepName(unsigned index, ResourceStatus&& status);
    void updateState();

    void markAllUnchecked();
    void markUnchecked(unsigned index);

    template <typename ListT>
    void clearAllAndPopulateNames(const ListT& list);

    std::u8string name(unsigned index) const;
    ResourceState state(unsigned index) const;
    unsigned compileId(unsigned index) const;

    ResourceState listState() const;

    template <typename Function>
    void readResourceListState(Function f) const
    {
        std::shared_lock lock(_mutex);

        // Ensure `f` has read-only access to `_state` and `_resources`
        const ResourceState& const_state = _state;
        const std::vector<ResourceStatus>& const_resources = _resources;

        f(const_state, const_resources);
    }

    template <typename Function>
    void readResourceState(unsigned index, Function f) const
    {
        std::shared_lock lock(_mutex);

        if (index < _resources.size()) {
            // Ensure `f` has read-only access to `_resources`
            const ResourceStatus& const_status = _resources.at(index);
            f(const_status);
        }
    }
};

template <typename T>
class DataStore final : public ResourceListStatus {
    std::unordered_map<std::u8string, size_t> _mapping;
    std::vector<std::shared_ptr<const T>> _data;

public:
    DataStore(std::u8string typeNameSingle, std::u8string typeNamePlural);

    // MUST include a lock in each function
    // MUST NOT implement a write functions in the header file

    [[nodiscard]] std::pair<std::optional<unsigned>, std::shared_ptr<const T>> indexAndDataFor(const idstring& id) const
    {
        std::shared_lock lock(_mutex);

        auto it = _mapping.find(id.str());
        if (it != _mapping.end()) {
            return { it->second, _data.at(it->second) };
        }
        else {
            return { std::nullopt, nullptr };
        }
    }

    [[nodiscard]] std::shared_ptr<const T> at(unsigned index) const
    {
        std::shared_lock lock(_mutex);

        if (index < _data.size()) {
            return _data.at(index);
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

    // Returns the old name and current name of the resource
    std::pair<std::u8string, std::u8string> store(const size_t index, ResourceStatus&& status, std::shared_ptr<const T>&& data);
};

class ProjectSettingsData {
private:
    mutable std::shared_mutex _mutex;

    std::shared_ptr<const Scripting::GameStateData> _gameState;
    std::shared_ptr<const Scripting::BytecodeMapping> _bytecode;
    std::shared_ptr<const MetaSprite::ActionPointMapping> _actionPointMapping;
    std::shared_ptr<const MetaTiles::InteractiveTilesData> _interactiveTiles;
    std::shared_ptr<const Resources::CompiledScenesData> _scenes;
    std::shared_ptr<const Entity::CompiledEntityRomData> _entityRomData;

public:
    // MUST include a lock in each function
    // MUST NOT implement a write functions in the header file

    std::shared_ptr<const Scripting::GameStateData> gameState() const
    {
        std::shared_lock lock(_mutex);
        return _gameState;
    }

    std::shared_ptr<const Scripting::BytecodeMapping> bytecode() const
    {
        std::shared_lock lock(_mutex);
        return _bytecode;
    }

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

    void store(std::shared_ptr<const Scripting::GameStateData>&& data);
    void store(std::shared_ptr<const Scripting::BytecodeMapping>&& data);
    void store(std::shared_ptr<const MetaSprite::ActionPointMapping>&& data);
    void store(std::shared_ptr<const MetaTiles::InteractiveTilesData>&& data);
    void store(std::shared_ptr<const Resources::CompiledScenesData>&& data);
    void store(std::shared_ptr<const Entity::CompiledEntityRomData>&& data);
};

class ProjectDependencies {
private:
    struct Mappings {
        std::vector<idstring> preresquite;

        // Mapping of resource name -> indexes of items that depend on the name
        std::unordered_multimap<std::u8string, unsigned> dependants;
    };

private:
    mutable std::shared_mutex _mutex;

    Mappings _exportOrder_frameSet;

    Mappings _palette_backgroundImage;
    Mappings _palette_metaTileTilesets;

public:
    ProjectDependencies() = default;

    void createDependencyGraph(const ProjectFile& project);
    void updateDependencyGraph(const ProjectFile& project, const ResourceType type, const unsigned index);

    void markProjectSettingsDepenantsUnchecked(ProjectData& projectData, ProjectSettingsIndex index) const;
    void markDependantsUnchecked(ProjectData& projectData, const ResourceType type, const std::u8string& oldName, const std::u8string& name);
};

class ProjectData {
private:
    friend class ProjectDependencies;

    ProjectDependencies _dependencies;

    ResourceListStatus _projectSettingsStatus;
    ResourceListStatus _frameSetExportOrderStatus;

    ProjectSettingsData _projectSettingsData;

    DataStore<UnTech::MetaSprite::Compiler::FrameSetData> _frameSets;
    DataStore<Resources::PaletteData> _palettes;
    DataStore<Resources::BackgroundImageData> _backgroundImages;
    DataStore<MetaTiles::MetaTileTilesetData> _metaTileTilesets;
    DataStore<Rooms::RoomData> _rooms;

    std::array<std::reference_wrapper<ResourceListStatus>, N_RESOURCE_TYPES> _resourceListStatuses;

public:
    ProjectData(const ProjectData&) = delete;
    ProjectData(ProjectData&&) = delete;
    ProjectData& operator=(const ProjectData&) = delete;
    ProjectData& operator=(ProjectData&&) = delete;

    ~ProjectData() = default;

public:
    ProjectData();

    const auto& projectSettingsStatus() const { return _projectSettingsStatus; }
    const auto& frameSetExportOrderStatus() const { return _frameSetExportOrderStatus; }

    const DataStore<UnTech::MetaSprite::Compiler::FrameSetData>& frameSets() const { return _frameSets; }
    const DataStore<Resources::PaletteData>& palettes() const { return _palettes; }
    const DataStore<Resources::BackgroundImageData>& backgroundImages() const { return _backgroundImages; }
    const DataStore<MetaTiles::MetaTileTilesetData>& metaTileTilesets() const { return _metaTileTilesets; }
    const DataStore<Rooms::RoomData>& rooms() const { return _rooms; }

    std::shared_ptr<const Scripting::GameStateData> gameState() const { return _projectSettingsData.gameState(); }
    std::shared_ptr<const MetaTiles::InteractiveTilesData> interactiveTiles() const { return _projectSettingsData.interactiveTiles(); }
    std::shared_ptr<const Resources::CompiledScenesData> scenes() const { return _projectSettingsData.scenes(); }
    std::shared_ptr<const Entity::CompiledEntityRomData> entityRomData() const { return _projectSettingsData.entityRomData(); }
    std::shared_ptr<const Scripting::BytecodeMapping> bytecodeData() const { return _projectSettingsData.bytecode(); }

    const ResourceListStatus& resourceListStatus(const ResourceType type) const { return _resourceListStatuses.at(static_cast<unsigned>(type)); }

    // To be called when the resource list changes
    void clearAndPopulateNamesAndDependencies(const ProjectFile& project);

    void updateDependencyGraph(const ProjectFile& project, const ResourceType type, const unsigned index);

    void markAllResourcesInvalid();
    void markResourceInvalid(const ResourceType type, const unsigned index);

    bool compileAll_EarlyExit(const ProjectFile& project) { return compileAll(project, true); }
    bool compileAll_NoEarlyExit(const ProjectFile& project) { return compileAll(project, false); }

private:
    bool compileAll(const ProjectFile& project, const bool earlyExit);

    template <typename DataT, typename ConvertFunction, class InputT, typename... PreresquitesT>
    bool compilePs(const ProjectSettingsIndex indexEnum, ConvertFunction convertFunction, const InputT& input, const PreresquitesT&... prerequisites);

    template <typename ValidateFunction, class InputT>
    bool validatePs(const ProjectSettingsIndex indexEnum, const ValidateFunction validateFunction, const InputT& input);

    template <typename ValidateFunction, class InputT>
    bool validateList(const ValidateFunction validateFunction, ResourceListStatus& statusList, const ResourceType type,
                      const ExternalFileList<InputT>& inputList);

    template <typename DataT, typename ConvertFunction, class InputListT, typename... PreresquitesT>
    bool compileList(ConvertFunction convertFunction, DataStore<DataT>& dataStore, const ResourceType type,
                     const InputListT& inputList,
                     const PreresquitesT&... prerequisites);
};

}
