/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "item-index.h"
#include "models/project/project-data.h"
#include "models/project/project-file-mutex.h"
#include "models/project/project.h"
#include "windows/projectlist.h"
#include <atomic>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <vector>

namespace UnTech::Gui {
class AbstractEditorData;
class AbstractEditorGui;

class BackgroundThread {
private:
    UnTech::Project::ProjectFileMutex& projectFile;
    UnTech::Project::ProjectData& projectData;

    std::thread thread;

    std::mutex mutex;
    std::condition_variable cv;

    std::atomic_bool threadActive;
    std::atomic_bool pendingAction;

    std::atomic_bool isProcessing;

    std::atomic_flag projectDataValid;

    std::atomic_bool markAllResourcesInvalidFlag;
    std::vector<ItemIndex> markResourceInvalidQueue;

public:
    BackgroundThread(UnTech::Project::ProjectFileMutex& pf,
                     UnTech::Project::ProjectData& pd);
    ~BackgroundThread();

    void markProjectListsInvalid();
    void markResourceInvalid(ItemIndex index);

private:
    void run();
};

class UnTechEditor {
private:
    static std::shared_ptr<UnTechEditor> _instance;

    UnTech::Project::ProjectFileMutex _projectFile;
    UnTech::Project::ProjectData _projectData;

    BackgroundThread _backgroundThread;

    const std::filesystem::path _filename;
    const std::string _basename;

    std::vector<std::unique_ptr<AbstractEditorGui>> _editorGuis;

    std::vector<std::unique_ptr<AbstractEditorData>> _editors;
    AbstractEditorData* _currentEditor;
    AbstractEditorGui* _currentEditorGui;

    ProjectListWindow _projectListWindow;

    bool _openUnsavedChangesOnExitPopup;
    bool _editorExited;
    std::vector<std::string> _unsavedFilesList;

private:
    UnTechEditor(std::unique_ptr<UnTech::Project::ProjectFile>&& pf,
                 const std::filesystem::path& fn);

public:
    // May be null
    static std::shared_ptr<UnTechEditor> instance() { return _instance; }

    // Only one project can be loaded per exectable.
    static void newProject(const std::filesystem::path& filename);
    static void loadProject(const std::filesystem::path& filename);
    static void closeProject();

    std::optional<ItemIndex> selectedItemIndex() const;

    void requestExitEditor();
    bool editorExited() const { return _editorExited; }

    void processGui();

    // called after ImGUI render
    void updateProjectFile();

private:
    void openEditor(const Project::ProjectFile& pf, const ItemIndex itemIndex);
    void closeEditor(const Project::ProjectFile& pf);

    bool saveProjectFile(const Project::ProjectFile& pf);
    bool saveEditor(const Project::ProjectFile& pf, AbstractEditorData* editor);
    bool saveAll(const Project::ProjectFile& pf);

    void processMenu(const Project::ProjectFile& pf);

    void unsavedChangesOnExitPopup(const Project::ProjectFile& pf);
};

}
