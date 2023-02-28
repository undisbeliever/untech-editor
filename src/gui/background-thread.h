/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2023, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "item-index.h"
#include "models/common/mutex_wrapper.h"
#include "models/project/project.h"
#include <memory>
#include <thread>

namespace UnTech::Project {
struct ProjectFile;
struct ProjectData;
class CompilerStatus;
}

namespace UnTech::Gui {

using ProjectFileMutex = shared_mutex<std::unique_ptr<UnTech::Project::ProjectFile>>;

class BackgroundThread {
public:
    struct ChangesQueue {
        bool resourceListMovedOrResized;
        std::vector<ItemIndex> resources;
    };

private:
    // All fields in this class must be thread safe
    mutex<ChangesQueue> queue;
    std::binary_semaphore queueChanged;

    const ProjectFileMutex& projectFile;
    Project::ProjectData& projectData;
    Project::CompilerStatus& compilerStatus;

    std::jthread thread;

public:
    BackgroundThread(const ProjectFileMutex& pf, Project::ProjectData& data, Project::CompilerStatus& status);

    void markResourceUnchecked(ItemIndex index);

    // Must be called when a resource list changes size or is reordered
    void markResourceListMovedOrResized();
};

}
