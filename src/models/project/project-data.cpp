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

static std::array<std::string, 5> projectSettingNames{
    "Project Settings",
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
    , _state()
    , _resources()
{
}

void ResourceListStatus::clearAllAndResize(size_t size)
{
    std::lock_guard lock(_mutex);

    _resources.clear();
    _resources.resize(size);
}

void ResourceListStatus::setStatus(unsigned index, ResourceStatus&& status)
{
    std::lock_guard lock(_mutex);

    _resources.at(index) = std::move(status);
}

void ResourceListStatus::setStatusKeepName(unsigned index, ResourceStatus&& status)
{
    std::lock_guard lock(_mutex);

    auto& s = _resources.at(index);

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

template <typename ListT>
void ResourceListStatus::clearAllAndPopulateNames(const ListT& list)
{
    std::lock_guard lock(_mutex);

    _resources.clear();
    _resources.resize(list.size());

    for (unsigned i = 0; i < list.size(); i++) {
        _resources.at(i).name = itemNameString(list, i);
    }
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
}

template <typename T>
inline void DataStore<T>::store(const size_t index, ResourceStatus&& status, std::shared_ptr<const T>&& data)
{
    std::lock_guard lock(_mutex);

    // ::TODO handle renamed items::

    _data.at(index) = std::move(data);

    const idstring& name = status.name;

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
    }

    _resources.at(index) = std::move(status);
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

// Compiling Functions
// ===================

template <typename ConvertFunction, class DataT, class InputT, typename... PreresquitesT>
static bool compileData(ConvertFunction convertFunction, DataStore<DataT>& dataStore,
                        const size_t index, const InputT& input,
                        const PreresquitesT&... preresquites)
{
    // Ensure dataStore is not a preresquite.
    assert(((static_cast<const void*>(&preresquites) != static_cast<const void*>(&dataStore)) && ...));

    ResourceStatus status;
    status.name = itemNameString(input);

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

    dataStore.store(index, std::move(status), std::move(data));

    return valid;
}

template <typename ConvertFunction, class DataT, class InputListT, typename... PreresquitesT>
static bool compileListItem(ConvertFunction convertFunction, DataStore<DataT>& dataStore,
                            const InputListT& inputList, const unsigned index,
                            const PreresquitesT&... preresquites)
{
    const auto& input = inputList.at(index);
    return compileData(convertFunction, dataStore, index, input, preresquites...);
}

template <typename ConvertFunction, class DataT, class InputT, typename... PreresquitesT>
static bool compileListItem(ConvertFunction convertFunction, DataStore<DataT>& dataStore,
                            const ExternalFileList<InputT>& inputList, const unsigned index,
                            const PreresquitesT&... preresquites)
{
    const auto* input = inputList.at(index);

    bool valid = true;

    if (input) {
        valid = compileData(convertFunction, dataStore, index, *input, preresquites...);
    }
    else {
        ResourceStatus status;
        status.state = ResourceState::Missing;
        status.name = inputList.item(index).filename.filename().string();
        status.errorList.addErrorString("External file is missing");

        dataStore.store(index, std::move(status), nullptr);

        valid = false;
    }

    return valid;
}

template <typename ConvertFunction, class DataT, class InputListT, typename... PreresquitesT>
static bool compileList(ConvertFunction convertFunction, DataStore<DataT>& dataStore,
                        const InputListT& inputList,
                        const PreresquitesT&... prerequisites)
{
    if (dataStore.size() != inputList.size()) {
        dataStore.clearAllAndResize(inputList.size());
    }

    bool valid = true;

    if ((checkPrerequisite(prerequisites) && ...)) {
        for (unsigned index = 0; index < inputList.size(); index++) {
            valid &= compileListItem(convertFunction, dataStore, inputList, index, prerequisites...);
        }
    }
    else {
        for (unsigned index = 0; index < inputList.size(); index++) {
            ResourceStatus status;
            status.state = ResourceState::Invalid;
            status.name = itemNameString(inputList, index);
            status.errorList.addErrorString("Dependency error");
            dataStore.store(index, std::move(status), nullptr);
        }
        valid = false;
    }

    dataStore.updateState();

    return valid;
}

template <typename DataT, typename ConvertFunction, class InputT, typename... PreresquitesT>
static ResourceStatus compileFunction(ConvertFunction convertFunction, ProjectSettingsData& projectSettingsData,
                                      const InputT& input, const PreresquitesT&... prerequisites)
{
    ResourceStatus status;

    if (!(checkPrerequisite(prerequisites) && ...)) {
        status.state = ResourceState::Invalid;
        status.errorList.addErrorString("Dependency error");
        return status;
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

    projectSettingsData.store(std::move(data));

    return status;
}

template <typename ValidateFunction, class InputT>
static ResourceStatus validateFunction(const ValidateFunction validateFunction, const InputT& input)
{
    ResourceStatus status;

    try {
        const bool valid = (input.*validateFunction)(status.errorList);
        status.state = valid ? ResourceState::Valid : ResourceState::Invalid;
    }
    catch (const std::exception& ex) {
        status.state = ResourceState::Invalid;
        status.errorList.addErrorString(stringBuilder("EXCEPTION: ", ex.what()));
    }

    return status;
}

template <typename ValidateFunction, class InputT>
static bool validateList(const ValidateFunction validateFunction, ResourceListStatus& statusList,
                         const ExternalFileList<InputT>& inputList)
{
    statusList.clearAllAndResize(inputList.size());

    bool valid = true;

    for (unsigned index = 0; index < inputList.size(); index++) {
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

        statusList.setStatus(index, std::move(status));
    }

    statusList.updateState();

    return valid;
}

ProjectData::ProjectData()
    : _projectSettingsStatus("", "Project Settings")
    , _frameSetExportOrderStatus("FrameSet Export Order", "FrameSet Export Orders")
    , _frameSets("FrameSet", "FrameSets")
    , _palettes("Palette", "Palettes")
    , _backgroundImages("Background Image", "Background Images")
    , _metaTileTilesets("MetaTile Tileset", "MetaTile Tileset")
    , _rooms("Room", "Rooms")
{
    _projectSettingsStatus.clearAllAndPopulateNames(projectSettingNames);
}

bool ProjectData::storePsStatus(ProjectSettingsIndex indexEnum, ResourceStatus&& newStatus)
{
    const unsigned index = static_cast<unsigned>(indexEnum);

    const bool valid = newStatus.state == ResourceState::Valid;

    _projectSettingsStatus.setStatusKeepName(index, std::move(newStatus));

    return valid;
}

bool ProjectData::compileAll(const ProjectFile& project, const bool earlyExit)
{
    bool valid = true;

    valid &= storePsStatus(ProjectSettingsIndex::ProjectSettings,
                           validateFunction(&ProjectSettings::validate, project.projectSettings));

    valid &= validateList(&MetaSprite::FrameSetExportOrder::validate, _frameSetExportOrderStatus, project.frameSetExportOrders);

    valid &= compileList(Resources::convertPalette, _palettes, project.palettes);

    valid &= storePsStatus(ProjectSettingsIndex::ActionPoints,
                           compileFunction<MetaSprite::ActionPointMapping>(MetaSprite::generateActionPointMapping, _projectSettingsData, project.actionPointFunctions));

    valid &= storePsStatus(ProjectSettingsIndex::InteractiveTiles,
                           compileFunction<MetaTiles::InteractiveTilesData>(MetaTiles::convertInteractiveTiles, _projectSettingsData, project.interactiveTiles));

    if (earlyExit && !valid) {
        return false;
    }

    valid &= compileList(MetaSprite::Compiler::compileFrameSet, _frameSets, project.frameSets, project, _projectSettingsData.actionPointMapping());

    valid &= compileList(Resources::convertBackgroundImage, _backgroundImages, project.backgroundImages, _palettes);

    valid &= compileList(MetaTiles::convertTileset, _metaTileTilesets, project.metaTileTilesets, _palettes, _projectSettingsData.interactiveTiles());

    if (earlyExit && !valid) {
        return false;
    }

    valid &= storePsStatus(ProjectSettingsIndex::EntityRomData,
                           compileFunction<Entity::CompiledEntityRomData>(Entity::compileEntityRomData, _projectSettingsData, project.entityRomData, project));

    valid &= storePsStatus(ProjectSettingsIndex::Scenes,
                           compileFunction<Resources::CompiledScenesData>(Resources::compileScenesData, _projectSettingsData, project.resourceScenes, *this));

    if (earlyExit && !valid) {
        return false;
    }

    valid &= compileList(Rooms::compileRoom, _rooms, project.rooms, _projectSettingsData.scenes(), _projectSettingsData.entityRomData(), project.projectSettings.roomSettings);

    _projectSettingsStatus.updateState();

    return valid;
}

}
}
