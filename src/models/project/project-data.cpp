/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "project-data.h"
#include "project.h"
#include "models/common/errorlist.h"
#include "models/common/type-traits.h"
#include "models/metasprite/compiler/framesetcompiler.h"
#include <cassert>

namespace UnTech {
namespace Project {

static const idstring BLANK_ID{};

static std::array<std::string, 7> projectSettingNames{
    "Project Settings",
    "Game State",
    "Bytecode",
    "Interactive Tiles",
    "Action Points",
    "Entities",
    "Scenes",
};

// Template Magic
// ==============

template <class T>
static const idstring& itemNameString(const T& item)
{
    return item.name;
}
static const idstring& itemNameString(const MetaSprite::FrameSetFile& item)
{
    return item.name();
}

template <class ListT>
static const std::string& itemNameString(const ListT& list, unsigned index)
{
    return list.at(index).name;
}
template <class T>
static std::string itemNameString(const ExternalFileList<T>& list, unsigned index)
{
    const ExternalFileItem<T>& item = list.item(index);
    if (item.value) {
        return item.value->name;
    }
    else {
        return item.filename.filename().string();
    }
}
template <size_t N>
static const std::string& itemNameString(const std::array<std::string, N>& list, unsigned index)
{
    return list.at(index);
}
static const idstring& itemNameString(const std::vector<MetaSprite::FrameSetFile>& list, unsigned index)
{
    return list.at(index).name();
}

template <typename T>
static inline bool checkPrerequisite(const std::shared_ptr<T>& p)
{
    return p != nullptr;
}
template <typename T>
static inline bool checkPrerequisite(const std::optional<T>& p)
{
    return p.has_value();
}
template <typename T>
static inline bool checkPrerequisite(const T&)
{
    return true;
}

template <typename T>
static inline const DataStore<T>& expandPresquite(const DataStore<T>& ds)
{
    return ds;
}
template <typename T>
static inline const T& expandPresquite(const std::shared_ptr<T>& p)
{
    assert(p);
    return *p;
}
template <typename T>
static inline const T& expandPresquite(const std::optional<T>& p)
{
    assert(p);
    return *p;
}
template <typename T>
static inline const T& expandPresquite(const T& p)
{
    return p;
}

// Status/Data methods
// ==================
// All "write" methods MUST be implemented in the .cpp file and protected with a `std::unqiue_lock` or `std::lock_guard`

ResourceListStatus::ResourceListStatus(std::string tnSingle, std::string tnPlural)
    : _typeNameSingle(tnSingle)
    , _typeNamePlural(tnPlural)
    , _currentCompileId(1)
    , _state(ResourceState::Unchecked)
    , _resources()
{
}

void ResourceListStatus::clearAllAndResize(size_t size)
{
    std::lock_guard lock(_mutex);

    _resources.clear();
    _resources.resize(size);

    _state = ResourceState::Unchecked;
}

inline std::pair<std::string, std::string> ResourceListStatus::setStatus(unsigned index, ResourceStatus&& status)
{
    std::lock_guard lock(_mutex);

    status.compileId = _currentCompileId++;

    std::string oldName = _resources.at(index).name;
    std::string newName = status.name;

    _resources.at(index) = std::move(status);

    return { oldName, newName };
}

void ResourceListStatus::setStatusKeepName(unsigned index, ResourceStatus&& status)
{
    std::lock_guard lock(_mutex);

    auto& s = _resources.at(index);

    s.compileId = _currentCompileId++;

    s.state = status.state;
    s.errorList = std::move(status.errorList);
}

void ResourceListStatus::updateState()
{
    std::lock_guard lock(_mutex);

    bool valid = std::all_of(_resources.cbegin(), _resources.cend(),
                             [](const auto& rs) { return rs.state == ResourceState::Valid; });

    _state = valid ? ResourceState::Valid : ResourceState::Invalid;
}

void ResourceListStatus::markAllUnchecked()
{
    std::lock_guard lock(_mutex);

    _state = ResourceState::Unchecked;
    for (auto& rs : _resources) {
        rs.state = ResourceState::Unchecked;
    }
}

void ResourceListStatus::markUnchecked(unsigned index)
{
    std::lock_guard lock(_mutex);

    _state = ResourceState::Unchecked;
    _resources.at(index).state = ResourceState::Unchecked;
}

template <typename ListT>
void ResourceListStatus::clearAllAndPopulateNames(const ListT& list)
{
    std::lock_guard lock(_mutex);

    _resources.clear();
    _resources.resize(list.size());

    for (unsigned i = 0; i < list.size(); i++) {
        _resources.at(i).name = itemNameString(list, i);
    }

    _state = ResourceState::Unchecked;
}

inline std::string ResourceListStatus::name(unsigned index) const
{
    std::shared_lock lock(_mutex);

    if (index < _resources.size()) {
        return _resources.at(index).name;
    }
    else {
        return {};
    }
}

ResourceState ResourceListStatus::state(unsigned index) const
{
    std::shared_lock lock(_mutex);

    if (index < _resources.size()) {
        return _resources.at(index).state;
    }
    else {
        return ResourceState::Missing;
    }
}

unsigned ResourceListStatus::compileId(unsigned index) const
{
    std::shared_lock lock(_mutex);

    if (index < _resources.size()) {
        return _resources.at(index).compileId;
    }
    else {
        return UINT_MAX;
    }
}

ResourceState ResourceListStatus::listState() const
{
    std::shared_lock lock(_mutex);

    return _state;
}

template <typename T>
DataStore<T>::DataStore(std::string typeNameSingle, std::string typeNamePlural)
    : ResourceListStatus(std::move(typeNameSingle), std::move(typeNamePlural))
    , _mapping()
    , _data()
{
}

template <typename T>
void DataStore<T>::clearAllAndResize(size_t size)
{
    std::lock_guard lock(_mutex);

    _resources.clear();
    _resources.resize(size);

    _mapping.clear();
    _mapping.reserve(size);
    _data.clear();
    _data.resize(size);

    _state = ResourceState::Unchecked;
}

template <typename T>
inline std::pair<std::string, std::string> DataStore<T>::store(const size_t index, ResourceStatus&& status, std::shared_ptr<const T>&& data)
{
    std::lock_guard lock(_mutex);

    status.compileId = _currentCompileId++;

    _data.at(index) = std::move(data);

    const std::string oldName = _resources.at(index).name;
    const std::string newName = status.name;

    const idstring name = status.name;

    if (name.isValid()) {
        auto it = _mapping.find(name);
        if (it == _mapping.end()) {
            _mapping.emplace(name, index);
        }
        else {
            if (it->second != index) {
                status.state = ResourceState::Invalid;
                status.errorList.addErrorString("Duplicated name detected");
            }
        }

        // Remove old mapping if the item has been renamed
        if (oldName != newName) {
            const auto oldIt = _mapping.find(oldName);
            if (oldIt != _mapping.end()) {
                if (oldIt->second == index) {
                    _mapping.erase(oldIt);
                }
            }
        }
    }

    _resources.at(index) = std::move(status);

    return { oldName, newName };
}

void ProjectSettingsData::store(std::shared_ptr<const Scripting::GameStateData>&& data)
{
    std::lock_guard lock(_mutex);

    _gameState = std::move(data);
}

void ProjectSettingsData::store(std::shared_ptr<const Scripting::BytecodeMapping>&& data)
{
    std::lock_guard lock(_mutex);

    _bytecode = std::move(data);
}

void ProjectSettingsData::store(std::shared_ptr<const MetaSprite::ActionPointMapping>&& data)
{
    std::lock_guard lock(_mutex);

    _actionPointMapping = std::move(data);
}

void ProjectSettingsData::store(std::shared_ptr<const MetaTiles::InteractiveTilesData>&& data)
{
    std::lock_guard lock(_mutex);

    _interactiveTiles = std::move(data);
}

void ProjectSettingsData::store(std::shared_ptr<const Resources::CompiledScenesData>&& data)
{
    std::lock_guard lock(_mutex);

    _scenes = std::move(data);
}

void ProjectSettingsData::store(std::shared_ptr<const Entity::CompiledEntityRomData>&& data)
{
    std::lock_guard lock(_mutex);

    _entityRomData = std::move(data);
}

// ProjectDataDependencies
// =======================

static const idstring& getExportOrder(const MetaSprite::FrameSetFile& fs)
{
    if (fs.siFrameSet) {
        return fs.siFrameSet->exportOrder;
    }
    else if (fs.msFrameSet) {
        return fs.msFrameSet->exportOrder;
    }
    else {
        return BLANK_ID;
    }
}

inline void ProjectDependencies::createDependencyGraph(const ProjectFile& project)
{
    std::lock_guard lock(_mutex);

    auto update = [&](Mappings& mappings, const auto& list, auto f) {
        mappings.dependants.clear();

        mappings.preresquite.clear();
        mappings.preresquite.reserve(list.size());

        for (unsigned i = 0; i < list.size(); i++) {
            const auto& item = list.at(i);
            const idstring name = f(item);

            mappings.dependants.emplace(name, i);
            mappings.preresquite.push_back(std::move(name));
        }
    };

    update(_exportOrder_frameSet, project.frameSets, getExportOrder);

    update(_palette_backgroundImage, project.backgroundImages,
           [](auto& bi) { return bi.conversionPlette; });

    update(_palette_metaTileTilesets, project.metaTileTilesets,
           [](auto* t) { return t ? t->animationFrames.conversionPalette : idstring{}; });
}

inline void ProjectDependencies::updateDependencyGraph(const ProjectFile& project, const ResourceType type, const unsigned index)
{
    std::lock_guard lock(_mutex);

    auto update = [&](Mappings& mappings, const idstring& newName) {
        const auto& oldName = mappings.preresquite.at(index);

        if (oldName != newName) {
            const auto range = mappings.dependants.equal_range(oldName);
            auto it = range.first;
            while (it != range.second) {
                if (it->second == index) {
                    it = mappings.dependants.erase(it);
                }
                else {
                    it++;
                }
            }

            mappings.preresquite.at(index) = newName;
            mappings.dependants.emplace(newName, index);
        }
    };

    switch (type) {
    case ResourceType::ProjectSettings: {
        break;
    }

    case ResourceType::FrameSetExportOrders: {
        break;
    }

    case ResourceType::FrameSets: {
        auto& fs = project.frameSets.at(index);
        update(_exportOrder_frameSet, getExportOrder(fs));

        break;
    }

    case ResourceType::Palettes: {
        break;
    }

    case ResourceType::BackgroundImages: {
        const auto& bi = project.backgroundImages.at(index);
        update(_palette_backgroundImage, bi.conversionPlette);

        break;
    }

    case ResourceType::MataTileTilesets: {
        const auto* tileset = project.metaTileTilesets.at(index);
        const auto& pal = tileset ? tileset->animationFrames.conversionPalette : BLANK_ID;
        update(_palette_metaTileTilesets, pal);

        break;
    }

    case ResourceType::Rooms: {
        break;
    }
    }
}

void ProjectDependencies::markProjectSettingsDepenantsUnchecked(ProjectData& projectData, ProjectSettingsIndex index)
{
    std::shared_lock lock(_mutex);

    switch (index) {
    case ProjectSettingsIndex::ProjectSettings:
        break;

    case ProjectSettingsIndex::GameState:
        projectData._rooms.markAllUnchecked();
        break;

    case ProjectSettingsIndex::Bytecode:
        projectData._rooms.markAllUnchecked();
        break;

    case ProjectSettingsIndex::InteractiveTiles:
        projectData._metaTileTilesets.markAllUnchecked();
        break;

    case ProjectSettingsIndex::ActionPoints:
        projectData._frameSets.markAllUnchecked();
        break;

    case ProjectSettingsIndex::EntityRomData:
        projectData._rooms.markAllUnchecked();
        projectData._projectSettingsStatus.markUnchecked(int(ProjectSettingsIndex::GameState));
        break;

    case ProjectSettingsIndex::Scenes:
        projectData._rooms.markAllUnchecked();
        break;
    }
}

void ProjectDependencies::markDependantsUnchecked(ProjectData& projectData, const ResourceType type,
                                                  const std::string& oldName, const std::string& name)
{
    std::shared_lock lock(_mutex);

    auto markPsUnchecked = [&](ProjectSettingsIndex psi) {
        projectData._projectSettingsStatus.markUnchecked(unsigned(psi));
    };

    auto markNameUnchecked = [](const Mappings& mappings, ResourceListStatus& rls, const idstring& n) {
        const auto range = mappings.dependants.equal_range(n);
        for (auto it = range.first; it != range.second; ++it) {
            rls.markUnchecked(it->second);
        }
    };
    auto markUnchecked = [&](const Mappings& mappings, ResourceListStatus& rls) {
        markNameUnchecked(mappings, rls, BLANK_ID);
        markNameUnchecked(mappings, rls, name);
        if (oldName != name) {
            markNameUnchecked(mappings, rls, oldName);
        }
    };

    switch (type) {
    case ResourceType::ProjectSettings: {
        break;
    }

    case ResourceType::FrameSetExportOrders: {
        markUnchecked(_exportOrder_frameSet, projectData._frameSets);
        break;
    }

    case ResourceType::FrameSets: {
        markPsUnchecked(ProjectSettingsIndex::EntityRomData);
        break;
    }

    case ResourceType::Palettes: {
        markUnchecked(_palette_backgroundImage, projectData._backgroundImages);
        markUnchecked(_palette_metaTileTilesets, projectData._metaTileTilesets);
        markPsUnchecked(ProjectSettingsIndex::Scenes);
        break;
    }

    case ResourceType::BackgroundImages: {
        markPsUnchecked(ProjectSettingsIndex::Scenes);
        break;
    }

    case ResourceType::MataTileTilesets: {
        markPsUnchecked(ProjectSettingsIndex::Scenes);
        break;
    }

    case ResourceType::Rooms: {
        break;
    }
    }
}

// Compiling Functions
// ===================

template <typename DataT, typename ConvertFunction, class InputT, typename... PreresquitesT>
static std::pair<ResourceStatus, std::shared_ptr<const DataT>>
compileData(ConvertFunction convertFunction, const InputT& input, const PreresquitesT&... preresquites)
{
    const std::string name = itemNameString(input);

    ResourceStatus status;
    status.name = name;

    bool valid = false;
    std::shared_ptr<const DataT> data = nullptr;

    try {
        data = convertFunction(input, expandPresquite(preresquites)..., status.errorList);

        if (data) {
            valid = data->validate(status.errorList);
        }
    }
    catch (const std::exception& ex) {
        status.errorList.addErrorString(stringBuilder("EXCEPTION: ", ex.what()));
        valid = false;
        data = nullptr;
    }

    if (!valid) {
        data = nullptr;
    }
    status.state = valid ? ResourceState::Valid : ResourceState::Invalid;

    return { std::move(status), std::move(data) };
}

template <typename DataT, typename ConvertFunction, class InputT, typename... PreresquitesT>
static std::pair<ResourceStatus, std::shared_ptr<const DataT>>
compileListItem(ConvertFunction convertFunction,
                const std::vector<InputT>& inputList, const unsigned index,
                const PreresquitesT&... preresquites)
{
    const auto& input = inputList.at(index);
    return compileData<DataT>(convertFunction, input, preresquites...);
}

template <typename DataT, typename ConvertFunction, class InputT, typename... PreresquitesT>
static std::pair<ResourceStatus, std::shared_ptr<const DataT>>
compileListItem(ConvertFunction convertFunction,
                const NamedList<InputT>& inputList, const unsigned index,
                const PreresquitesT&... preresquites)
{
    const auto& input = inputList.at(index);
    return compileData<DataT>(convertFunction, input, preresquites...);
}

template <typename DataT, typename ConvertFunction, class InputT, typename... PreresquitesT>
static std::pair<ResourceStatus, std::shared_ptr<const DataT>>
compileListItem(ConvertFunction convertFunction,
                const ExternalFileList<InputT>& inputList, const unsigned index,
                const PreresquitesT&... preresquites)
{
    const auto* input = inputList.at(index);

    if (input) {
        return compileData<DataT>(convertFunction, *input, preresquites...);
    }
    else {
        ResourceStatus status;
        status.state = ResourceState::Missing;
        status.name = inputList.item(index).filename.filename().string();
        status.errorList.addErrorString("External file is missing");

        return { std::move(status), nullptr };
    }
}

template <typename DataT, typename ConvertFunction, class InputT, typename... PreresquitesT>
static std::pair<ResourceStatus, std::shared_ptr<const DataT>>
compileFunction(ConvertFunction convertFunction,
                const InputT& input, const PreresquitesT&... prerequisites)
{
    ResourceStatus status;

    if (!(checkPrerequisite(prerequisites) && ...)) {
        status.state = ResourceState::Invalid;
        status.errorList.addErrorString("Dependency error");
        return { std::move(status), nullptr };
    }

    std::shared_ptr<const DataT> data;

    try {
        data = convertFunction(input, expandPresquite(prerequisites)..., status.errorList);
        status.state = data ? ResourceState::Valid : ResourceState::Invalid;
    }
    catch (const std::exception& ex) {
        data = nullptr;
        status.state = ResourceState::Invalid;
        status.errorList.addErrorString(stringBuilder("EXCEPTION: ", ex.what()));
    }

    return { std::move(status), std::move(data) };
}

template <typename DataT, typename ConvertFunction, class InputT, typename... PreresquitesT>
bool ProjectData::compilePs(const ProjectSettingsIndex indexEnum, ConvertFunction convertFunction,
                            const InputT& input, const PreresquitesT&... prerequisites)
{
    const unsigned index = static_cast<unsigned>(indexEnum);

    if (_projectSettingsStatus.state(index) != ResourceState::Unchecked) {
        return true;
    }

    auto [status, data] = compileFunction<DataT>(convertFunction, input, prerequisites...);

    const bool valid = status.state == ResourceState::Valid;

    _projectSettingsData.store(std::move(data));
    _projectSettingsStatus.setStatusKeepName(index, std::move(status));

    _dependencies.markProjectSettingsDepenantsUnchecked(*this, indexEnum);

    return valid;
}

template <typename ValidateFunction, class InputT>
bool ProjectData::validatePs(const ProjectSettingsIndex indexEnum, const ValidateFunction validateFunction, const InputT& input)
{
    const unsigned index = static_cast<unsigned>(indexEnum);

    if (_projectSettingsStatus.state(index) != ResourceState::Unchecked) {
        return true;
    }

    ResourceStatus status;

    try {
        const bool valid = (input.*validateFunction)(status.errorList);
        status.state = valid ? ResourceState::Valid : ResourceState::Invalid;
    }
    catch (const std::exception& ex) {
        status.state = ResourceState::Invalid;
        status.errorList.addErrorString(stringBuilder("EXCEPTION: ", ex.what()));
    }

    const bool valid = status.state == ResourceState::Valid;

    _projectSettingsStatus.setStatusKeepName(index, std::move(status));

    _dependencies.markProjectSettingsDepenantsUnchecked(*this, indexEnum);

    return valid;
}

template <typename ValidateFunction, class InputT>
inline bool ProjectData::validateList(const ValidateFunction validateFunction, ResourceListStatus& statusList,
                                      const ResourceType type, const ExternalFileList<InputT>& inputList)
{
    if (statusList.listState() != ResourceState::Unchecked) {
        return true;
    }

    statusList.clearAllAndResize(inputList.size());

    bool valid = true;

    for (unsigned index = 0; index < inputList.size(); index++) {
        if (statusList.state(index) == ResourceState::Unchecked) {
            const auto* input = inputList.at(index);

            ResourceStatus status;

            if (input) {
                const bool v = (input->*validateFunction)(status.errorList);
                status.state = v ? ResourceState::Valid : ResourceState::Invalid;
                status.name = itemNameString(*input);

                valid &= v;
            }
            else {
                status.state = ResourceState::Missing;
                status.name = inputList.item(index).filename.filename().string();
                status.errorList.addErrorString("External file is missing");

                valid = false;
            }

            auto [oldName, name] = statusList.setStatus(index, std::move(status));
            _dependencies.markDependantsUnchecked(*this, type, oldName, name);
        }
    }

    statusList.updateState();

    return valid;
}

template <typename DataT, typename ConvertFunction, class InputListT, typename... PreresquitesT>
inline bool ProjectData::compileList(ConvertFunction convertFunction, DataStore<DataT>& dataStore, const ResourceType type,
                                     const InputListT& inputList,
                                     const PreresquitesT&... prerequisites)
{
    // Ensure dataStore is not a preresquite.
    assert(((static_cast<const void*>(&prerequisites) != static_cast<const void*>(&dataStore)) && ...));

    if (dataStore.listState() != ResourceState::Unchecked) {
        return true;
    }

    if (dataStore.size() != inputList.size()) {
        dataStore.clearAllAndResize(inputList.size());
    }

    bool valid = true;

    if ((checkPrerequisite(prerequisites) && ...)) {
        for (unsigned index = 0; index < inputList.size(); index++) {
            if (dataStore.state(index) == ResourceState::Unchecked) {
                auto [status, data] = compileListItem<DataT>(convertFunction, inputList, index, prerequisites...);
                valid &= status.state == ResourceState::Valid;

                auto [oldName, name] = dataStore.store(index, std::move(status), std::move(data));
                _dependencies.markDependantsUnchecked(*this, type, oldName, name);
            }
        }
    }
    else {
        for (unsigned index = 0; index < inputList.size(); index++) {
            if (dataStore.state(index) == ResourceState::Unchecked) {
                ResourceStatus status;
                status.state = ResourceState::Invalid;
                status.name = itemNameString(inputList, index);
                status.errorList.addErrorString("Dependency error");

                auto [oldName, name] = dataStore.store(index, std::move(status), nullptr);
                _dependencies.markDependantsUnchecked(*this, type, oldName, name);
            }
        }
        valid = false;
    }

    dataStore.updateState();

    return valid;
}

ProjectData::ProjectData()
    : _dependencies()
    , _projectSettingsStatus("Project Settings", "Project Settings")
    , _frameSetExportOrderStatus("FrameSet Export Order", "FrameSet Export Orders")
    , _frameSets("FrameSet", "FrameSets")
    , _palettes("Palette", "Palettes")
    , _backgroundImages("Background Image", "Background Images")
    , _metaTileTilesets("MetaTile Tileset", "MetaTile Tileset")
    , _rooms("Room", "Rooms")
    , _resourceListStatuses{
        _projectSettingsStatus,
        _frameSetExportOrderStatus,
        _frameSets,
        _palettes,
        _backgroundImages,
        _metaTileTilesets,
        _rooms,
    }
{
    _projectSettingsStatus.clearAllAndPopulateNames(projectSettingNames);
}

void ProjectData::clearAndPopulateNamesAndDependencies(const ProjectFile& project)
{
    _dependencies.createDependencyGraph(project);

    _projectSettingsStatus.markAllUnchecked();
    _frameSetExportOrderStatus.clearAllAndPopulateNames(project.frameSetExportOrders);
    _frameSets.clearAllAndPopulateNames(project.frameSets);
    _palettes.clearAllAndPopulateNames(project.palettes);
    _backgroundImages.clearAllAndPopulateNames(project.backgroundImages);
    _metaTileTilesets.clearAllAndPopulateNames(project.metaTileTilesets);
    _rooms.clearAllAndPopulateNames(project.rooms);
}

void ProjectData::updateDependencyGraph(const ProjectFile& project, const ResourceType type, const unsigned index)
{
    _dependencies.updateDependencyGraph(project, type, index);
}

void ProjectData::markAllResourcesInvalid()
{
    for (auto& r : _resourceListStatuses) {
        r.get().markAllUnchecked();
    }
}

void ProjectData::markResourceInvalid(const ResourceType type, const unsigned index)
{
    _resourceListStatuses.at(static_cast<unsigned>(type)).get().markUnchecked(index);
}

bool ProjectData::compileAll(const ProjectFile& project, const bool earlyExit)
{
    bool valid = true;

    // ::TODO mark GameState unchecked if room name changes::

    valid &= validatePs(ProjectSettingsIndex::ProjectSettings,
                        &ProjectSettings::validate, project.projectSettings);

    valid &= compilePs<Scripting::BytecodeMapping>(ProjectSettingsIndex::Bytecode,
                                                   Scripting::compileBytecode, project.bytecode);

    valid &= validateList(&MetaSprite::FrameSetExportOrder::validate, _frameSetExportOrderStatus,
                          ResourceType::FrameSetExportOrders, project.frameSetExportOrders);

    valid &= compileList(Resources::convertPalette, _palettes, ResourceType::Palettes, project.palettes);

    valid &= compilePs<MetaSprite::ActionPointMapping>(ProjectSettingsIndex::ActionPoints,
                                                       MetaSprite::generateActionPointMapping, project.actionPointFunctions);

    valid &= compilePs<MetaTiles::InteractiveTilesData>(ProjectSettingsIndex::InteractiveTiles,
                                                        MetaTiles::convertInteractiveTiles, project.interactiveTiles);

    if (earlyExit && !valid) {
        return false;
    }

    valid &= compileList(MetaSprite::Compiler::compileFrameSet, _frameSets, ResourceType::FrameSets,
                         project.frameSets, project, _projectSettingsData.actionPointMapping());

    valid &= compileList(Resources::convertBackgroundImage, _backgroundImages, ResourceType::BackgroundImages,
                         project.backgroundImages, _palettes);

    valid &= compileList(MetaTiles::convertTileset, _metaTileTilesets, ResourceType::MataTileTilesets,
                         project.metaTileTilesets, _palettes, _projectSettingsData.interactiveTiles());

    if (earlyExit && !valid) {
        return false;
    }

    valid &= compilePs<Entity::CompiledEntityRomData>(ProjectSettingsIndex::EntityRomData,
                                                      Entity::compileEntityRomData, project.entityRomData, project);

    valid &= compilePs<Scripting::GameStateData>(ProjectSettingsIndex::GameState,
                                                 Scripting::compileGameState, project.gameState, project.rooms, project.entityRomData);

    valid &= compilePs<Resources::CompiledScenesData>(ProjectSettingsIndex::Scenes,
                                                      Resources::compileScenesData, project.resourceScenes, *this);

    if (earlyExit && !valid) {
        return false;
    }

    valid &= compileList(Rooms::compileRoom, _rooms, ResourceType::Rooms,
                         project.rooms, project.rooms, _projectSettingsData.scenes(), _projectSettingsData.entityRomData(), project.projectSettings.roomSettings,
                         _projectSettingsData.gameState(), _projectSettingsData.bytecode());

    _projectSettingsStatus.updateState();

    return valid;
}

}
}
