/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2023, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "item-index.h"
#include "models/common/mutex_wrapper.h"
#include "models/project/compiler-status.h"
#include "models/project/project-data.h"
#include "models/project/project.h"
#include <atomic>
#include <memory>
#include <thread>

namespace UnTech::Project {
struct ProjectFile;
struct ProjectData;
class CompilerStatus;
}

namespace UnTech::Gui {

class BackgroundThread {
public:
    struct ChangesQueue {
        bool resourceListMovedOrResized;
        std::vector<ItemIndex> resources;
    };

private:
    Project::CompilerStatus _compilerStatus;
    Project::ProjectData _projectData;

    shared_mutex<std::unique_ptr<UnTech::Project::ProjectFile>> projectFile;

    // All fields in this class must be thread safe
    mutex<ChangesQueue> queue;
    std::binary_semaphore queueChanged;

    std::atomic_flag cancelToken;

    std::jthread thread;

public:
    // pf MUST NOT be nullptr
    explicit BackgroundThread(std::unique_ptr<Project::ProjectFile> pf);

    void markResourceUnchecked(ItemIndex index);

    // Must be called when a resource list changes size or is reordered
    void markResourceListMovedOrResized();

    const auto& compilerStatus() const { return _compilerStatus; }
    const auto& projectData() const { return _projectData; }

    template <typename Function>
    void read_pf(Function f) { projectFile.read(f); }

    template <typename Function>
    void tryWrite_pf(Function f)
    {
        // Cancel BG thread (if it is still active)
        cancelToken.test_and_set();

        projectFile.tryWrite(f);

        // Resume BG thread (if required)
        queueChanged.release();
    }

    template <typename Function>
    void write_pf(Function f)
    {
        // Cancel BG thread (if it is still active)
        cancelToken.test_and_set();

        projectFile.write(f);

        // Resume BG thread (if required)
        queueChanged.release();
    }
};

}
