/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "background-thread.h"
#include "item-index.h"
#include "splitter.h"
#include "models/project/project.h"
#include "windows/projectlist.h"
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace UnTech::Gui {
class AbstractEditorData;
class AbstractExternalFileEditorData;
class AbstractEditorGui;

class UnTechEditor {
private:
    static std::shared_ptr<UnTechEditor> _instance;

    BackgroundThread _backgroundThread;

    const std::filesystem::path _filename;
    const std::u8string _basename;

    std::vector<gsl::not_null<std::shared_ptr<AbstractEditorGui>>> _editorGuis;

    std::vector<gsl::not_null<std::shared_ptr<AbstractEditorData>>> _editors;

    std::shared_ptr<AbstractEditorData> _currentEditor;
    std::shared_ptr<AbstractEditorGui> _currentEditorGui;

    // The last compileId read for the current resource
    uint64_t _lastCompileId;

    ProjectListWindow _projectListWindow;

    SplitterBarState _projectListSidebar;

    bool _showProjectListSidebar;
    bool _openUnsavedChangesOnExitPopup;
    bool _editorExited;

    std::vector<std::u8string> _unsavedFilesList;

private:
    // pf MUST NOT be nullptr
    UnTechEditor(std::unique_ptr<UnTech::Project::ProjectFile> pf,
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
