/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>
#include <shared_mutex>

namespace UnTech::Project {

struct ProjectFile;

class ProjectFileMutex {
    mutable std::shared_mutex _mutex;
    std::unique_ptr<ProjectFile> const _project;

private:
    ProjectFileMutex(const ProjectFileMutex&) = delete;
    ProjectFileMutex(ProjectFileMutex&&) = delete;
    ProjectFileMutex& operator=(const ProjectFileMutex&) = delete;
    ProjectFileMutex& operator=(ProjectFileMutex&&) = delete;

public:
    ProjectFileMutex(std::unique_ptr<ProjectFile> project)
        : _mutex()
        , _project(std::move(project))
    {
        if (_project == nullptr) {
            throw std::invalid_argument("project cannot be null");
        }
    }

    template <typename Function>
    void read(Function f) const
    {
        std::shared_lock lock(_mutex);

        f(const_cast<const ProjectFile&>(*_project));
    }

    template <typename Function>
    void write(Function f)
    {
        std::lock_guard lock(_mutex);

        f(*_project);
    }

    template <typename Function>
    void tryWrite(Function f)
    {
        std::unique_lock lock(_mutex, std::try_to_lock);

        if (lock.owns_lock()) {
            f(*_project);
        }
    }
};

}
