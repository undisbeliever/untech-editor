/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2023, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "compiler-status.h"
#include "project.h"
#include "models/common/iterators.h"
#include "models/enums.h"
#include <atomic>

namespace UnTech::Project {

[[nodiscard]] static uint64_t getNextCompileId()
{
    static std::atomic<uint64_t> compileId = 0;

    return ++compileId;
}

static constexpr std::array<std::u8string_view, 7> projectSettingNames{
    u8"Project Settings",
    u8"Game State",
    u8"Bytecode",
    u8"Interactive Tiles",
    u8"Action Points",
    u8"Entities",
    u8"Scenes",
};

template <typename T>
static inline void resizeListAndPopulateNames(CompilerStatus::ListData& listData, const NamedList<T>& list)
{
    listData.state = ResourceState::AllUnchecked;
    listData.resources.clear();
    listData.resources.resize(list.size());

    for (const auto [i, item] : enumerate(list)) {
        listData.resources.at(i).name = item.name.str();
    }
}

template <typename T>
static inline void resizeListAndPopulateNames(CompilerStatus::ListData& listData, const ExternalFileList<T>& list)
{
    listData.state = ResourceState::AllUnchecked;
    listData.resources.clear();
    listData.resources.resize(list.size());

    for (const auto [i, item] : enumerate(list)) {
        auto& rs = listData.resources.at(i);

        if (item.value) {
            rs.name = item.value->name.str();
        }
        else {
            rs.name = item.filename.filename().u8string();
        }
    }
}

static inline void resizeListAndPopulateNames(CompilerStatus::ListData& listData, const std::vector<MetaSprite::FrameSetFile>& list)
{
    listData.state = ResourceState::AllUnchecked;
    listData.resources.clear();
    listData.resources.resize(list.size());

    for (const auto [i, item] : enumerate(list)) {
        auto& rs = listData.resources.at(i);

        if (item.siFrameSet) {
            rs.name = item.siFrameSet->name.str();
        }
        else if (item.msFrameSet) {
            rs.name = item.msFrameSet->name.str();
        }
        else {
            rs.name = item.filename.filename().u8string();
        }
    }
}

inline CompilerStatus::ListData::ListData(std::u8string tnSingle, std::u8string tnPlural)
    : typeNameSingle(std::move(tnSingle))
    , typeNamePlural(std::move(tnPlural))
    , state(ResourceState::AllUnchecked)
    , resources()
{
}

CompilerStatus::CompilerStatus(const ProjectFile& project)
    : _resourceLists({ {
        { u8"Project Settings", u8"Project Settings" },
        { u8"FrameSet Export Order", u8"FrameSet Export Orders" },
        { u8"FrameSet", u8"FrameSets" },
        { u8"Palette", u8"Palettes" },
        { u8"Background Image", u8"Background Images" },
        { u8"MetaTile Tileset", u8"MetaTile Tilesets" },
        { u8"Room", u8"Rooms" },
    } })
{
    _resourceLists.write([](auto& rl) {
        auto& ps = rl.at(size_t(ResourceType::ProjectSettings));

        ps.resources.resize(projectSettingNames.size());

        for (auto [i, r] : enumerate(ps.resources)) {
            r.name = projectSettingNames.at(i);
        }
    });

    updateListSizeAndNames(project);
}

void CompilerStatus::updateListSizeAndNames(const ProjectFile& pf)
{
    _resourceLists.write([&](auto& rl) {
        auto getList = [&](ResourceType t) -> ListData& { return rl.at(size_t(t)); };

        auto& ps = getList(ResourceType::ProjectSettings);
        ps.state = ResourceState::AllUnchecked;
        for (auto& r : ps.resources) {
            r.state = ResourceState::Unchecked;
        }

        resizeListAndPopulateNames(getList(ResourceType::FrameSetExportOrders), pf.frameSetExportOrders);
        resizeListAndPopulateNames(getList(ResourceType::FrameSets), pf.frameSets);
        resizeListAndPopulateNames(getList(ResourceType::Palettes), pf.palettes);
        resizeListAndPopulateNames(getList(ResourceType::BackgroundImages), pf.backgroundImages);
        resizeListAndPopulateNames(getList(ResourceType::MataTileTilesets), pf.metaTileTilesets);
        resizeListAndPopulateNames(getList(ResourceType::Rooms), pf.rooms);
    });
}

void CompilerStatus::markAllUnchecked()
{
    _resourceLists.write([&](auto& rl) {
        for (auto& ps : rl) {
            ps.state = ResourceState::AllUnchecked;

            for (auto& r : ps.resources) {
                r.state = ResourceState::Unchecked;
            }
        }
    });
}

static inline void updateDependencies(std::array<CompilerStatus::ListData, N_RESOURCE_TYPES>& resourceLists,
                                      ProjectSettingsIndex index)
{
    using RT = ResourceType;
    using PSI = ProjectSettingsIndex;

    const auto markPsUnchecked = [&](PSI i) {
        auto& ps = resourceLists.at(size_t(RT::ProjectSettings));
        ps.state = ResourceState::Unchecked;
        ps.resources.at(size_t(i)).state = ResourceState::Unchecked;
    };

    const auto markListUnchecked = [&](RT t) {
        auto& rl = resourceLists.at(size_t(t));
        rl.state = ResourceState::AllUnchecked;
        for (auto& r : rl.resources) {
            r.state = ResourceState::Unchecked;
        }
    };

    // NOLINTBEGIN(bugprone-branch-clone)

    switch (index) {
    case PSI::ProjectSettings:
        break;

    case PSI::GameState:
        markListUnchecked(RT::Rooms);
        break;

    case PSI::Bytecode:
        markListUnchecked(RT::Rooms);
        break;

    case PSI::InteractiveTiles:
        markListUnchecked(RT::MataTileTilesets);
        break;

    case PSI::ActionPoints:
        markListUnchecked(RT::FrameSets);
        break;

    case PSI::EntityRomData:
        markListUnchecked(RT::Rooms);
        markPsUnchecked(PSI::GameState);
        break;

    case PSI::Scenes:
        markListUnchecked(RT::Rooms);
        break;
    }

    // NOLINTEND(bugprone-branch-clone)
}

static inline void updateDependencies(std::array<CompilerStatus::ListData, N_RESOURCE_TYPES>& resourceLists,
                                      const ResourceType type, const idstring& resName, const ProjectFile& pf)
{
    using RT = ResourceType;
    using PSI = ProjectSettingsIndex;

    const auto markPsUnchecked = [&](PSI i) {
        auto& ps = resourceLists.at(size_t(RT::ProjectSettings));
        ps.state = ResourceState::Unchecked;
        ps.resources.at(size_t(i)).state = ResourceState::Unchecked;
    };

    const auto markUnchecked = [&](RT t, size_t i) {
        auto& rl = resourceLists.at(size_t(t));
        rl.state = ResourceState::Unchecked;
        rl.resources.at(i).state = ResourceState::Unchecked;
    };

    const auto markListUnchecked = [&](RT t) {
        auto& rl = resourceLists.at(size_t(t));
        rl.state = ResourceState::AllUnchecked;
        for (auto& r : rl.resources) {
            r.state = ResourceState::Unchecked;
        }
    };

    // NOLINTBEGIN(bugprone-branch-clone)

    switch (type) {
    case RT::ProjectSettings: {
        break;
    }

    case RT::FrameSetExportOrders: {
        if (resName.isValid()) {
            for (auto [i, fs] : enumerate(pf.frameSets)) {
                if (fs.siFrameSet) {
                    if (fs.siFrameSet->exportOrder == resName) {
                        markUnchecked(RT::FrameSets, i);
                    }
                }
                else if (fs.msFrameSet) {
                    if (fs.msFrameSet->exportOrder == resName) {
                        markUnchecked(RT::FrameSets, i);
                    }
                }
            }
        }
        markPsUnchecked(PSI::EntityRomData);
        markListUnchecked(RT::Rooms);
        break;
    }

    case RT::FrameSets: {
        markPsUnchecked(PSI::EntityRomData);
        markListUnchecked(RT::Rooms);
        break;
    }

    case RT::Palettes: {
        if (resName.isValid()) {
            for (auto [i, bi] : enumerate(pf.backgroundImages)) {
                if (bi.conversionPlette == resName) {
                    markUnchecked(RT::BackgroundImages, i);
                }
            }
            for (auto [i, mt] : enumerate(pf.metaTileTilesets)) {
                if (mt.value) {
                    const auto it = std::find(mt.value->palettes.begin(), mt.value->palettes.end(), resName);
                    if (it != mt.value->palettes.end()) {
                        markUnchecked(RT::MataTileTilesets, i);
                    }
                }
            }
        }

        markPsUnchecked(PSI::Scenes);
        markListUnchecked(RT::Rooms);
        break;
    }

    case RT::BackgroundImages: {
        markPsUnchecked(PSI::Scenes);
        markListUnchecked(RT::Rooms);
        break;
    }

    case RT::MataTileTilesets: {
        markPsUnchecked(PSI::Scenes);

        // ::TODO reduce number of recompiled rooms::
        markListUnchecked(RT::Rooms);
        break;
    }

    case RT::Rooms: {
        break;
    }
    }

    // NOLINTEND(bugprone-branch-clone)
}

[[nodiscard]] static const idstring& getName(const ProjectFile& pf, const ResourceType type, const size_t index)
{

    static constexpr idstring BLANK_IDSTRING{};

    using RT = ResourceType;

    switch (type) {
    case RT::ProjectSettings:
        break;

    case RT::FrameSetExportOrders: {
        const auto eo = pf.frameSetExportOrders.at(index);
        if (eo) {
            return eo->name;
        }
        break;
    }

    case RT::FrameSets: {
        return pf.frameSets.at(index).name();
    }

    case RT::Palettes: {
        return pf.palettes.at(index).name;
    }

    case RT::BackgroundImages: {
        return pf.backgroundImages.at(index).name;
    }

    case RT::MataTileTilesets: {
        const auto mt = pf.metaTileTilesets.at(index);
        if (mt) {
            return mt->name;
        }
        break;
    }

    case RT::Rooms: {
        const auto rm = pf.rooms.at(index);
        if (rm) {
            return rm->name;
        }
        break;
    }
    }

    return BLANK_IDSTRING;
}

void CompilerStatus::markUnchecked(const ResourceType type, const size_t index, const ProjectFile& pf)
{
    if (type == ResourceType::ProjectSettings) {
        _resourceLists.write([&](auto& rl) {
            auto& rlist = rl.at(size_t(type));
            rlist.state = ResourceState::Unchecked;
            if (index < rlist.resources.size()) {
                updateDependencies(rl, ProjectSettingsIndex(index));
            }
        });
    }
    else {
        const idstring& name = getName(pf, type, index);

        _resourceLists.write([&](auto& rl) {
            auto& rlist = rl.at(size_t(type));

            rlist.state = ResourceState::Unchecked;

            if (index < rlist.resources.size()) {
                auto& entry = rlist.resources.at(index);
                entry.state = ResourceState::Unchecked;

                if (entry.name != name.str()) {
                    // Mark resources using the old name Unchecked.
                    updateDependencies(rl, type, idstring::fromString(entry.name), pf);
                    entry.name = name.str();
                }
                updateDependencies(rl, type, name, pf);
            }
        });
    }
}

bool CompilerStatus::updateResourceListState(const ResourceType type)
{
    return _resourceLists.write_and_return_bool([&](auto& rl) {
        auto& listStatus = rl.at(size_t(type));

        const bool valid = std::all_of(listStatus.resources.begin(), listStatus.resources.end(),
                                       [](const auto& r) { return r.state == ResourceState::Valid; });
        listStatus.state = valid ? ResourceState::Valid : ResourceState::Invalid;

        return valid;
    });
}

void CompilerStatus::store(const ResourceType type, const size_t index, ResourceState state, ErrorList&& errorList)
{
    _resourceLists.write([&](auto& rl) {
        auto& listStatus = rl.at(size_t(type));

        if (state != ResourceState::Valid && state != ResourceState::Unchecked) {
            listStatus.state = ResourceState::Invalid;
        }

        auto& rs = listStatus.resources.at(index);
        rs.compileId = getNextCompileId();
        rs.state = state;
        rs.errorList = std::move(errorList);
    });
}

void CompilerStatus::dependencyErrorOnList(const ResourceType type)
{
    _resourceLists.write([&](auto& rl) {
        const auto cid = getNextCompileId();

        auto& listData = rl.at(size_t(type));

        listData.state = ResourceState::DependencyError;
        for (auto& rs : listData.resources) {
            rs.compileId = cid;
            rs.state = ResourceState::DependencyError;
            rs.errorList = ErrorList();
        }
    });
}

ResourceState CompilerStatus::getState(const ResourceType type) const
{
    return _resourceLists.read_and_return_enum<ResourceState>([&](const auto& rl) {
        return rl.at(size_t(type)).state;
    });
}

ResourceState CompilerStatus::getState(const ResourceType type, const size_t index) const
{
    return _resourceLists.read_and_return_enum<ResourceState>([&](const auto& rl) {
        auto& listStatus = rl.at(size_t(type));
        return listStatus.resources.at(index).state;
    });
}

uint64_t CompilerStatus::getCompileId(const ProjectSettingsIndex index) const
{
    return _resourceLists.read_and_return_uint64_t([&](const auto& rl) {
        auto& listStatus = rl.at(size_t(ResourceType::ProjectSettings));
        return listStatus.resources.at(size_t(index)).compileId;
    });
}

}
