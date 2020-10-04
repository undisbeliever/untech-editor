/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "project-data.h"
#include "project.h"
#include "models/common/errorlist.h"
#include "models/metasprite/compiler/framesetcompiler.h"
#include <cassert>

namespace UnTech {
namespace Project {

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

ResourceListStatus::ResourceListStatus(std::string tnSingle, std::string tnPlural)
    : typeNameSingle(tnSingle)
    , typeNamePlural(tnPlural)
    , state()
    , resources()
{
}

void ResourceListStatus::clearAllAndResize(size_t size)
{
    resources.clear();
    resources.resize(size);
}

void ResourceListStatus::updateState()
{
    bool valid = std::all_of(resources.cbegin(), resources.cend(),
                             [](const auto& rs) { return rs.state == ResourceState::Valid; });

    state = valid ? ResourceState::Valid : ResourceState::Invalid;
}

template <typename T>
DataStore<T>::DataStore(std::string typeNameSingle, std::string typeNamePlural)
    : _listStatus(std::move(typeNameSingle), std::move(typeNamePlural))
    , _mapping()
    , _data()
{
}

template <typename T>
inline void DataStore<T>::store(const size_t index, ResourceStatus&& status, std::shared_ptr<const T>&& data)
{
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

    _listStatus.resources.at(index) = std::move(status);
}

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

template <typename ConvertFunction, class DataT, class InputT, typename... PreresquitesT>
static ResourceStatus compileFunction(ConvertFunction convertFunction, std::shared_ptr<DataT>& data,
                                      const InputT& input, const PreresquitesT&... prerequisites)
{
    ResourceStatus status;

    if (!(checkPrerequisite(prerequisites) && ...)) {
        status.state = ResourceState::Invalid;
        status.errorList.addErrorString("Dependency error");
        return status;
    }

    try {
        data = convertFunction(input, expandPresquite(prerequisites)..., status.errorList);
        status.state = data ? ResourceState::Valid : ResourceState::Invalid;
    }
    catch (const std::exception& ex) {
        status.state = ResourceState::Invalid;
        status.errorList.addErrorString(stringBuilder("EXCEPTION: ", ex.what()));
        data = nullptr;
    }

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
    if (statusList.resources.size() != inputList.size()) {
        statusList.clearAllAndResize(inputList.size());
    }

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

        statusList.resources.at(index) = std::move(status);
    }

    statusList.updateState();

    return valid;
}

ProjectData::ProjectData(const ProjectFile& project)
    : _project(project)
    , _projectSettingsStatus("", "Project Settings")
    , _frameSetExportOrderStatus("FrameSet Export Order", "FrameSet Export Orders")
    , _frameSets("FrameSet", "FrameSets")
    , _palettes("Palette", "Palettes")
    , _backgroundImages("Background Image", "Background Images")
    , _metaTileTilesets("MetaTile Tileset", "MetaTile Tileset")
    , _rooms("Room", "Rooms")
    , _scenes(nullptr)
    , _entityRomData(nullptr)
{
    auto setName = [&](ProjectSettingsIndex index, const char* string) {
        _projectSettingsStatus.resources.at(static_cast<unsigned>(index)).name = string;
    };

    _projectSettingsStatus.resources.resize(N_PROJECT_SETTING_ITEMS);
    setName(ProjectSettingsIndex::MemoryMap, "Memory Map");
    setName(ProjectSettingsIndex::RoomSettings, "Room Settings");
    setName(ProjectSettingsIndex::InteractiveTiles, "Interactive Tiles");
    setName(ProjectSettingsIndex::ActionPoints, "Action Points");
    setName(ProjectSettingsIndex::EntityRomData, "Entities");
    setName(ProjectSettingsIndex::Scenes, "Scenes");
}

bool ProjectData::storePsStatus(ProjectSettingsIndex indexEnum, ResourceStatus&& newStatus)
{
    const unsigned index = static_cast<unsigned>(indexEnum);

    const bool valid = newStatus.state == ResourceState::Valid;

    auto& status = _projectSettingsStatus.resources.at(index);

    status.state = newStatus.state;
    status.errorList = std::move(newStatus.errorList);

    return valid;
}

bool ProjectData::compileAll()
{
    bool valid = true;

    valid &= storePsStatus(ProjectSettingsIndex::MemoryMap,
                           validateFunction(&MemoryMapSettings::validate, _project.memoryMap));

    valid &= storePsStatus(ProjectSettingsIndex::RoomSettings,
                           validateFunction(&Rooms::RoomSettings::validate, _project.roomSettings));

    valid &= validateList(&MetaSprite::FrameSetExportOrder::validate, _frameSetExportOrderStatus, _project.frameSetExportOrders);

    valid &= compileList(Resources::convertPalette, _palettes, _project.palettes);

    valid &= storePsStatus(ProjectSettingsIndex::ActionPoints,
                           compileFunction(MetaSprite::generateActionPointMapping, _actionPointMapping, _project.actionPointFunctions));

    valid &= storePsStatus(ProjectSettingsIndex::InteractiveTiles,
                           compileFunction(MetaTiles::convertInteractiveTiles, _interactiveTiles, _project.interactiveTiles));

    if (!valid) {
        return false;
    }

    valid &= compileList(MetaSprite::Compiler::compileFrameSet, _frameSets, _project.frameSets, _project, _actionPointMapping);

    valid &= compileList(Resources::convertBackgroundImage, _backgroundImages, _project.backgroundImages, _palettes);

    valid &= compileList(MetaTiles::convertTileset, _metaTileTilesets, _project.metaTileTilesets, _palettes, _interactiveTiles);

    if (!valid) {
        return false;
    }

    valid &= storePsStatus(ProjectSettingsIndex::EntityRomData,
                           compileFunction(Entity::compileEntityRomData, _entityRomData, _project.entityRomData, _project));

    valid &= storePsStatus(ProjectSettingsIndex::Scenes,
                           compileFunction(Resources::compileScenesData, _scenes, _project.resourceScenes, *this));

    if (!valid) {
        return false;
    }

    if (!valid) {
        return false;
    }

    valid &= compileList(Rooms::compileRoom, _rooms, _project.rooms, _scenes, _entityRomData, _project.roomSettings);

    _projectSettingsStatus.updateState();

    return valid;
}

}
}
