/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2023, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resource-compiler.h"
#include "compiler-status.h"
#include "project-data.h"
#include "project.h"
#include "models/common/externalfilelist.h"
#include "models/common/optional_ref.h"
#include "models/common/u8strings.h"
#include "models/metasprite/compiler/framesetcompiler.h"
#include <stdexcept>
#include <thread>
#include <unordered_set>

namespace UnTech::Project {

using RT = ResourceType;
using PSI = ProjectSettingsIndex;

static constexpr idstring BLANK_IDSTRING{};

template <typename T>
static inline optional_ref<const T&> getItem(const NamedList<T>& list, const size_t index)
{
    return list.at(index);
}

template <typename T>
static inline optional_ref<const T&> getItem(const ExternalFileList<T>& list, const size_t index)
{
    return list.at(index);
}

template <typename T>
static inline optional_ref<const T&> getItem(const std::vector<T>& list, const size_t index)
{
    return list.at(index);
}

template <typename T>
static inline const idstring& getItemName(const T& item)
    requires std::is_same_v<decltype(item.name), idstring>
{
    return item.name;
}

static inline const idstring& getItemName(const MetaSprite::FrameSetFile& item)
{
    return item.name();
}

static inline bool isUnchecked(const ResourceState rs)
{
    return rs == ResourceState::Unchecked || rs == ResourceState::AllUnchecked;
}

template <typename T>
static inline bool validateArg(const std::shared_ptr<const T>& arg)
{
    return arg != nullptr;
}

template <typename T>
static inline bool validateArg(const T&)
    requires std::is_class_v<T>
{
    return true;
}

template <typename... Args>
static inline bool validateArgs(const Args&... args)
{
    return (validateArg(args) && ...);
}

template <typename T>
static inline const T& expandArg(const std::shared_ptr<const T>& arg)
{
    if (!arg) {
        throw std::runtime_error("nullptr");
    }
    return *arg;
}

template <typename T>
static inline const T& expandArg(const T& arg)
    requires std::is_class_v<T>
{
    return arg;
}

template <typename Function, typename... Args>
static inline bool validatePs(CompilerStatus& status, const PSI psIndex,
                              Function validateFunction, const Args&... args)
    requires std::invocable<Function, const Args&..., ErrorList&>
{
    const auto index = size_t(psIndex);

    const auto oldState = status.getState(RT::ProjectSettings, index);
    if (isUnchecked(oldState)) {
        bool valid = false;
        ErrorList errorList;

        const bool dependenciesValid = validateArgs(args...);
        if (dependenciesValid) {
            try {
                valid = validateFunction(expandArg(args)..., errorList);
            }
            catch (const std::exception& ex) {
                valid = false;
                errorList.addErrorString(stringBuilder(u8"EXCEPTION: ", convert_old_string(ex.what())));
            }
        }
        else {
            errorList.addErrorString(u8"Dependency Error");
            valid = false;
        }

        const ResourceState state = valid ? ResourceState::Valid : ResourceState::Invalid;

        status.store(RT::ProjectSettings, index, state, std::move(errorList));

        return valid;
    }
    else {
        return oldState == ResourceState::Valid;
    }
}

template <typename Function, typename... Args>
static inline bool compilePs(CompilerStatus& status, const PSI psIndex,
                             ProjectSettingsData& dataStore,
                             Function compileFunction, const Args&... args)
    requires std::invocable<Function, const Args&..., ErrorList&>
{
    using ResultT = std::invoke_result_t<Function, const Args&..., ErrorList&>;

    const auto index = size_t(psIndex);

    const auto oldState = status.getState(RT::ProjectSettings, index);
    if (isUnchecked(oldState)) {
        ResultT data{ nullptr };
        ErrorList errorList;

        const bool dependenciesValid = validateArgs(args...);
        if (dependenciesValid) {
            try {
                data = compileFunction(expandArg(args)..., errorList);
            }
            catch (const std::exception& ex) {
                data = nullptr;
                errorList.addErrorString(stringBuilder(u8"EXCEPTION: ", convert_old_string(ex.what())));
            }
        }
        else {
            errorList.addErrorString(u8"Dependency Error");
        }

        const bool valid = data != nullptr;
        const ResourceState state = valid ? ResourceState::Valid : ResourceState::Invalid;

        status.store(RT::ProjectSettings, index, state, std::move(errorList));
        dataStore.store(std::move(data));

        return valid;
    }
    else {
        return oldState == ResourceState::Valid;
    }
}

template <typename Function, typename ListT, typename... Args>
static inline bool validateList(CompilerStatus& status, const RT type,
                                std::atomic_flag& cancelToken,
                                Function validateFunction, const ListT& list, const Args&... args)
{
    const auto oldListState = status.getState(type);
    if (isUnchecked(oldListState)) {
        const bool dependenciesValid = validateArgs(args...);
        if (not dependenciesValid) {
            status.dependencyErrorOnList(type);
            return false;
        }

        // Used to detect duplicate names
        std::unordered_set<idstring> names;

        const size_t listSize = list.size();

        for (const size_t index : range(listSize)) {
            if (cancelToken.test()) {
                return false;
            }

            const auto item = getItem(list, index);

            // Also adds the item name to the names set.
            // To detect duplicate names, every item must be checked.
            const bool itemNameValid = [&]() {
                if (item) {
                    const idstring& itemName = getItemName(*item);

                    if (itemName.isValid()) {
                        return names.insert(itemName).second;
                    }
                }
                return false;
            }();

            if (isUnchecked(status.getState(type, index))) {
                ResourceState state = ResourceState::Unchecked;
                ErrorList errorList;

                if (item) {
                    try {
                        const bool itemValid = validateFunction(*item, expandArg(args)..., errorList);
                        state = itemValid ? ResourceState::Valid : ResourceState::Invalid;
                    }
                    catch (const std::exception& ex) {
                        state = ResourceState::Invalid;
                        errorList.addErrorString(stringBuilder(u8"EXCEPTION: ", convert_old_string(ex.what())));
                    }

                    if (not itemNameValid) {
                        state = ResourceState::Invalid;
                        errorList.addErrorString(u8"Duplicate resource name: ", getItemName(*item));
                    }
                }
                else {
                    state = ResourceState::Missing;
                }

                status.store(type, index, state, std::move(errorList));
            }
        }

        return status.updateResourceListState(type);
    }
    else {
        return oldListState == ResourceState::Valid;
    }
}

template <typename Function, typename ListT, typename T, typename... Args>
static inline bool compileList(CompilerStatus& status, const RT type, DataStore<T>& dataStore,
                               std::atomic_flag& cancelToken,
                               Function compileFunction, const ListT& list, const Args&... args)
{
    const auto oldListState = status.getState(type);
    if (isUnchecked(oldListState)) {
        const bool dependenciesValid = validateArgs(args...);
        if (not dependenciesValid) {
            status.dependencyErrorOnList(type);
            dataStore.clearAllAndResize(list.size());
            return false;
        }

        const size_t listSize = list.size();

        if (oldListState == ResourceState::AllUnchecked) {
            dataStore.clearAllAndResize(listSize);
        }

        assert(dataStore.size() == list.size());

        for (const size_t index : range(listSize)) {
            if (cancelToken.test()) {
                return false;
            }

            if (isUnchecked(status.getState(type, index))) {
                std::shared_ptr<const T> data{ nullptr };
                ResourceState state = ResourceState::Unchecked;
                ErrorList errorList;

                const auto item = getItem(list, index);
                if (item) {
                    const idstring& itemName = getItemName(*item);

                    try {
                        data = compileFunction(*item, expandArg(args)..., errorList);
                        state = data != nullptr ? ResourceState::Valid : ResourceState::Invalid;
                    }
                    catch (const std::exception& ex) {
                        data = nullptr;
                        state = ResourceState::Invalid;
                        errorList.addErrorString(u8"EXCEPTION: ", convert_old_string(ex.what()));
                    }

                    const bool nameValid = dataStore.store(index, itemName, std::move(data));
                    if (not nameValid) {
                        state = ResourceState::Invalid;
                        errorList.addErrorString(u8"Duplicate resource name: ", itemName);
                    }
                }
                else {
                    state = ResourceState::Missing;

                    const bool nameValid = dataStore.store(index, BLANK_IDSTRING, nullptr);
                    (void)nameValid; // always false, no need to check it.
                }

                status.store(type, index, state, std::move(errorList));
            }
        }

        return status.updateResourceListState(type);
    }
    else {
        return oldListState == ResourceState::Valid;
    }
}

bool compileResources_impl(CompilerStatus& status, ProjectData& data, const ProjectFile& project, const bool earlyExit, std::atomic_flag& cancelToken)
{
    bool valid = true;

    valid &= validatePs(status, PSI::ProjectSettings,
                        Project::validateProjectSettings, project.projectSettings);

    valid &= compilePs(status, PSI::Bytecode,
                       data.projectSettingsData,
                       Scripting::compileBytecode, project.bytecode);

    if (cancelToken.test()) {
        return false;
    }

    valid &= validateList(status, RT::FrameSetExportOrders,
                          cancelToken,
                          MetaSprite::validateExportOrder, project.frameSetExportOrders);

    if (cancelToken.test()) {
        return false;
    }

    valid &= compileList(status, RT::Palettes,
                         data.palettes,
                         cancelToken,
                         Resources::convertPalette, project.palettes);

    if (cancelToken.test()) {
        return false;
    }

    valid &= compilePs(status, PSI::ActionPoints,
                       data.projectSettingsData,
                       MetaSprite::generateActionPointMapping, project.actionPointFunctions);

    if (cancelToken.test()) {
        return false;
    }

    valid &= compilePs(status, PSI::InteractiveTiles,
                       data.projectSettingsData,
                       MetaTiles::convertInteractiveTiles, project.interactiveTiles);

    if (earlyExit && !valid) {
        return false;
    }

    valid &= compileList(status, RT::FrameSets,
                         data.frameSets,
                         cancelToken,
                         MetaSprite::Compiler::compileFrameSet, project.frameSets, project, data.projectSettingsData.actionPointMapping());

    if (cancelToken.test()) {
        return false;
    }

    valid &= compileList(status, RT::BackgroundImages,
                         data.backgroundImages,
                         cancelToken,
                         Resources::convertBackgroundImage, project.backgroundImages, data.palettes);

    if (cancelToken.test()) {
        return false;
    }

    valid &= compileList(status, RT::MataTileTilesets,
                         data.metaTileTilesets,
                         cancelToken,
                         MetaTiles::convertTileset, project.metaTileTilesets, data.palettes, data.projectSettingsData.interactiveTiles());

    if (cancelToken.test()) {
        return false;
    }

    if (earlyExit && !valid) {
        return false;
    }

    valid &= compilePs(status, PSI::EntityRomData,
                       data.projectSettingsData,
                       Entity::compileEntityRomData, project.entityRomData, project);

    if (cancelToken.test()) {
        return false;
    }

    valid &= compilePs(status, PSI::GameState,
                       data.projectSettingsData,
                       Scripting::compileGameState, project.gameState, project.rooms, project.entityRomData);

    if (cancelToken.test()) {
        return false;
    }

    valid &= compilePs(status, PSI::Scenes,
                       data.projectSettingsData,
                       Resources::compileScenesData, project.resourceScenes, data);

    if (cancelToken.test()) {
        return false;
    }

    if (earlyExit && !valid) {
        return false;
    }

    valid &= compileList(status, RT::Rooms,
                         data.rooms,
                         cancelToken,
                         Rooms::compileRoom,
                         project.rooms, project.rooms, data.projectSettingsData.scenes(), data.projectSettingsData.entityRomData(),
                         project.projectSettings.roomSettings, data.projectSettingsData.gameState(), data.projectSettingsData.bytecodeData(), data.metaTileTilesets);

    if (cancelToken.test()) {
        return false;
    }

    status.updateResourceListState(RT::ProjectSettings);

    return valid;
}

}
