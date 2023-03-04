/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2023, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/errorlist.h"
#include "models/common/mutex_wrapper.h"
#include "models/enums.h"
#include <array>
#include <memory>
#include <vector>

namespace UnTech::Project {

struct ProjectFile;

enum class ResourceState {
    // If `ListData.status` is AllUnchecked, `compileResources()` will clear and resize the DataStore<T>>
    // ListData.status MUST be `AllUnchecked` if the list size changes.
    AllUnchecked,

    Unchecked,
    Valid,
    Invalid,
    Missing,
    DependencyError,
};

// This class is thread safe
class CompilerStatus {
public:
    struct ResourceStatus {
        // Used by the GUI to display resource name in the sidebar
        std::u8string name{};

        uint64_t compileId;

        ResourceState state = ResourceState::Unchecked;
        ErrorList errorList{};
    };

    struct ListData {
        const std::u8string typeNameSingle;
        const std::u8string typeNamePlural;

        ResourceState state;
        std::vector<ResourceStatus> resources;

        ListData(std::u8string tnSingle, std::u8string tnPlural);
    };

    struct Data {
    };

private:
    // All fields in this class must be thread safe.
    shared_mutex<std::array<ListData, N_RESOURCE_TYPES>> _resourceLists;

public:
    explicit CompilerStatus(const ProjectFile& project);

    const auto& resourceLists() const { return _resourceLists; }

    // MUST be called when an resource list is resized or reordered.
    // Will also recompile all resources
    void updateListSizeAndNames(const ProjectFile& pf);

    void markAllUnchecked();
    void markUnchecked(const ResourceType type, const size_t index, const ProjectFile& pf);

    void store(const ResourceType type, const size_t index, ResourceState state, ErrorList&& errorList);
    void dependencyErrorOnList(const ResourceType type);

    bool updateResourceListState(const ResourceType type);

    ResourceState getState(const ResourceType type) const;
    ResourceState getState(const ResourceType type, const size_t index) const;

    uint64_t getCompileId(const ProjectSettingsIndex index) const;

    template <typename Function>
    inline void readResourceState(const ResourceType type, const size_t index, Function f) const
        requires std::is_invocable_v<Function, const ResourceStatus&>
    {
        _resourceLists.read([&](const auto& rl) {
            auto& ld = rl.at(size_t(type));
            if (index < ld.resources.size()) {
                const ResourceStatus& rs = ld.resources.at(index);
                f(rs);
            }
        });
    }
};

}
