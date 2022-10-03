/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "item-index.h"
#include "splitter.h"
#include "gui/graphics/entity-graphics.h"
#include "models/project/project-data.h"
#include "models/project/project-file-mutex.h"
#include "models/project/project.h"
#include "windows/projectlist.h"
#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <vector>

namespace UnTech::Gui {
class AbstractEditorData;
class AbstractExternalFileEditorData;
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

    void markAllResourcesInvalid();
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
    const std::u8string _basename;

    std::vector<std::unique_ptr<AbstractEditorGui>> _editorGuis;

    std::vector<std::unique_ptr<AbstractEditorData>> _editors;
    AbstractEditorData* _currentEditor;
    AbstractEditorGui* _currentEditorGui;

    ProjectListWindow _projectListWindow;

    SplitterBarState _projectListSidebar;

    bool _openUnsavedChangesOnExitPopup;
    bool _editorExited;
    std::vector<std::u8string> _unsavedFilesList;

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

    void requestExitEditor();
    bool editorExited() const { return _editorExited; }

    void processGui();

    // called after ImGUI render
    void updateProjectFile();

    static void fixMissingBgWindow();

private:
    void openEditor(const ItemIndex itemIndex);
    void closeEditor();

    // Will block execution if background thread is reading ProjectFile
    void forceProcessEditorActions();

    bool saveEditor(AbstractExternalFileEditorData* editor);
    bool saveProjectFile();
    void saveCurrentEditor();
    bool saveAll();

    void invalidateImageCache();

    void processMenu();
    void processKeyboardShortcuts();

    void fullscreenBackgroundWindow();

    void unsavedChangesOnExitPopup();
};

}
