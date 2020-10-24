/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "untech-editor.h"
#include "abstract-editor.h"
#include "imgui-filebrowser.h"
#include "imgui.h"
#include "gui/windows/about-popup.h"
#include "gui/windows/error-list-window.h"
#include "gui/windows/message-box.h"
#include "gui/windows/projectlist.h"
#include "models/project/project.h"

namespace UnTech::Gui {

std::shared_ptr<UnTechEditor> UnTechEditor::_instance = nullptr;

UnTechEditor::UnTechEditor(std::unique_ptr<UnTech::Project::ProjectFile>&& pf, const std::filesystem::path& fn)
    : _projectFile(std::move(pf))
    , _projectData()
    , _backgroundThread(_projectFile, _projectData)
    , _filename(fn)
    , _basename(fn.filename())
    , _editorGuis(createEditorGuis())
    , _editors()
    , _currentEditor(nullptr)
    , _currentEditorGui(nullptr)
    , _projectListWindow()
    , _openUnsavedChangesOnExitPopup(false)
    , _editorExited(false)
    , _unsavedFilesList()
{
    if (_filename.empty()) {
        throw std::invalid_argument("filename cannot be empty");
    }
}

void UnTechEditor::newProject(const std::filesystem::path& fn)
{
    if (_instance) {
        return;
    }

    if (fn.empty()) {
        return;
    }

    try {
        // no need for an std::error_code, we are inside a try/catch block
        const auto filename = std::filesystem::absolute(fn).lexically_normal();

        if (std::filesystem::exists(filename)) {
            loadProject(filename);
            return;
        }

        auto pf = std::make_unique<UnTech::Project::ProjectFile>();
        UnTech::Project::saveProjectFile(*pf, filename);

        ImGui::setFileDialogDirectory(filename.parent_path());

        _instance = std::shared_ptr<UnTechEditor>(new UnTechEditor(std::move(pf), filename));
    }
    catch (const std::exception& ex) {
        MessageBox::showMessage("Cannot Create Project", ex.what());
    }
}

void UnTechEditor::loadProject(const std::filesystem::path& fn)
{
    if (_instance) {
        return;
    }

    if (fn.empty()) {
        return;
    }

    try {
        // no need for an std::error_code, we are inside a try/catch block
        const auto filename = std::filesystem::absolute(fn).lexically_normal();

        auto pf = UnTech::Project::loadProjectFile(filename);

        // ::TODO move into background thread::
        pf->loadAllFiles();

        ImGui::setFileDialogDirectory(filename.parent_path());

        _instance = std::shared_ptr<UnTechEditor>(new UnTechEditor(std::move(pf), filename));
    }
    catch (const std::exception& ex) {
        MessageBox::showMessage("Unable to Load Project", ex.what());
    }
}

void UnTechEditor::closeProject()
{
    if (_instance) {
        _instance.reset();
    }
}

std::optional<ItemIndex> UnTechEditor::selectedItemIndex() const
{
    if (_currentEditor) {
        return _currentEditor->itemIndex();
    }
    else {
        return std::nullopt;
    }
}

// MUST ONLY be called by `updateProjectFile`
void UnTechEditor::openEditor(const Project::ProjectFile& pf, const ItemIndex itemIndex)
{
    AbstractEditorData* editor = nullptr;

    auto it = std::find_if(_editors.cbegin(), _editors.cend(),
                           [&](auto& e) { return e->itemIndex() == itemIndex; });
    if (it != _editors.cend()) {
        editor = it->get();
    }

    if (editor == nullptr) {
        if (auto e = createEditor(itemIndex, pf)) {
            if (e->loadDataFromProject(pf)) {
                // itemIndex is valid
                editor = e.get();
                _editors.push_back(std::move(e));
            }
        }
    }

    if (editor != _currentEditor) {
        closeEditor(pf);
        _currentEditor = editor;
        if (editor) {
            const bool success = editor->loadDataFromProject(pf);
            if (success) {
                unsigned counter = 0;
                for (auto& eg : _editorGuis) {
                    if (eg->setEditorData(_currentEditor)) {
                        _currentEditorGui = eg.get();
                        counter++;
                    }
                }
                assert(counter == 1);
                assert(_currentEditorGui);

                _currentEditorGui->redoClicked = false;
                _currentEditorGui->undoClicked = false;

                _currentEditorGui->editorDataChanged();
                _currentEditorGui->editorOpened();
            }
            else {
                _currentEditor = nullptr;
                _currentEditorGui = nullptr;
            }
        }
    }
}

// MUST ONLY be called by `updateProjectFile`
void UnTechEditor::closeEditor(const Project::ProjectFile& pf)
{
    if (_currentEditor) {
        // Discard any uncommitted data
        _currentEditor->loadDataFromProject(pf);
    }
    if (_currentEditorGui) {
        _currentEditorGui->editorClosed();
    }

    _currentEditor = nullptr;
    _currentEditorGui = nullptr;
}

bool UnTechEditor::saveProjectFile(const Project::ProjectFile& pf)
{
    assert(!_filename.empty());

    try {
        UnTech::Project::saveProjectFile(pf, _filename);

        for (auto& e : _editors) {
            if (dynamic_cast<AbstractExternalFileEditorData*>(_currentEditor) == nullptr) {
                e->markClean();
            }
        }
        _projectListWindow.markClean();
        return true;
    }
    catch (const std::exception& ex) {
        MessageBox::showMessage("Cannot Save Project", ex.what());
        return false;
    }
}

bool UnTechEditor::saveEditor(const Project::ProjectFile& pf, AbstractEditorData* editor)
{
    if (auto e = dynamic_cast<AbstractExternalFileEditorData*>(editor)) {

        bool dataLoaded = e->loadDataFromProject(pf);
        assert(dataLoaded);

        assert(e->filename().empty() == false);
        try {
            e->saveFile();
            e->markClean();
            return true;
        }
        catch (const std::exception& ex) {
            MessageBox::showMessage("Cannot Save Resource", ex.what());
            return false;
        }
    }
    else {
        return saveProjectFile(pf);
    }
}

bool UnTechEditor::saveAll(const Project::ProjectFile& pf)
{
    bool ok = true;

    if (_projectListWindow.isClean() == false) {
        ok &= saveProjectFile(pf);
    }
    for (auto& e : _editors) {
        if (!e->isClean()) {
            ok &= saveEditor(pf, e.get());
        }
    }

    return ok;
}

void UnTechEditor::processMenu(const Project::ProjectFile& pf)
{
    using namespace std::string_literals;

    ImGui::BeginMainMenuBar();

    if (ImGui::BeginMenu("File")) {
        // ::TODO add Ctrl+S shortcut::
        if (_currentEditor) {
            if (!_currentEditor->basename().empty()) {
                auto s = "Save "s + _currentEditor->basename();
                if (ImGui::MenuItem(s.c_str())) {
                    saveEditor(pf, _currentEditor);
                }
            }
        }
        const auto saveProjectLabel = "Save "s + _basename;
        if (ImGui::MenuItem(saveProjectLabel.c_str())) {
            saveProjectFile(pf);
        }
        if (ImGui::MenuItem("Save All")) {
            saveAll(pf);
        }

        ImGui::Separator();

        _projectListWindow.processMenu();

        ImGui::Separator();

        if (ImGui::MenuItem("About UnTech Editor")) {
            AboutPopup::openPopup();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Exit")) {
            requestExitEditor();
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit")) {
        const bool canUndo = _currentEditor && _currentEditor->canUndo();
        const bool canRedo = _currentEditor && _currentEditor->canRedo();

        if (ImGui::MenuItem("Undo", nullptr, false, canUndo)) {
            if (_currentEditorGui) {
                _currentEditorGui->undoClicked = true;
            }
        }
        if (ImGui::MenuItem("Redo", nullptr, false, canRedo)) {
            if (_currentEditorGui) {
                _currentEditorGui->redoClicked = true;
            }
        }

        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

void UnTechEditor::requestExitEditor()
{
    const auto parentPath = _filename.parent_path();

    std::vector<std::string> files;
    files.push_back(_basename);

    bool projectFileClean = _projectListWindow.isClean();
    for (auto& e : _editors) {
        if (!e->isClean()) {
            if (auto* ee = dynamic_cast<AbstractExternalFileEditorData*>(e.get())) {
                files.push_back(ee->filename().lexically_relative(parentPath).u8string());
            }
            else {
                projectFileClean = false;
            }
        }
    }

    if (projectFileClean) {
        assert(!files.empty());
        files.erase(files.begin());
    }

    if (files.empty()) {
        _editorExited = true;
    }
    else {
        _editorExited = false;
        _openUnsavedChangesOnExitPopup = true;
        _unsavedFilesList = std::move(files);
    }
}

void UnTechEditor::unsavedChangesOnExitPopup(const Project::ProjectFile& pf)
{
    using namespace std::string_literals;
    const char* const windowTitle = "Save Changes?###UnsavedChangesPopup";

    static float buttonsWidth = 30;

    if (_openUnsavedChangesOnExitPopup) {
        _openUnsavedChangesOnExitPopup = false;
        ImGui::OpenPopup(windowTitle);
    }

    bool open = true;
    if (ImGui::BeginPopupModal(windowTitle, &open, ImGuiWindowFlags_NoResize)) {
        const auto& style = ImGui::GetStyle();

        if (_unsavedFilesList.size() == 1) {
            ImGui::TextUnformatted("There is one unsaved file.");
            ImGui::NewLine();
            ImGui::TextUnformatted("Do you wish to save?"s);
        }
        else {
            ImGui::Text("There are %d unsaved files.", int(_unsavedFilesList.size()));
            ImGui::NewLine();
            ImGui::TextUnformatted("Do you wish to save them all?");
        }

        ImGui::TextUnformatted("\n(Unsaved changes will be lost if you discard them)"s);

        ImGui::Spacing();
        {
            const float posX = (ImGui::GetWindowSize().x - buttonsWidth) / 2;
            ImGui::SetCursorPosX(posX);

            if (ImGui::Button("Save All")) {
                _editorExited = saveAll(pf);
            }
            ImGui::SameLine(0, style.IndentSpacing);
            if (ImGui::Button("Discard")) {
                _editorExited = true;
            }
            ImGui::SameLine(0, style.IndentSpacing);
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine(0, 0);
            buttonsWidth = ImGui::GetCursorPosX() - posX;

            ImGui::NewLine();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Indent();

        ImGui::SetNextItemWidth(-style.IndentSpacing);
        if (ImGui::ListBoxHeader("##UnsavedFiles", _unsavedFilesList.size(), 4)) {
            for (const auto& f : _unsavedFilesList) {
                ImGui::TextUnformatted(f);
            }
            ImGui::ListBoxFooter();
        }

        ImGui::Unindent();

        ImGui::EndPopup();
    }
}

void UnTechEditor::processGui()
{
    _projectFile.read([&](auto& pf) {
        if (_currentEditorGui) {
            assert(_currentEditor);

            _currentEditorGui->processGui(pf, _projectData);
            _currentEditor->updateSelection();

            processErrorListWindow(_projectData, _currentEditor);
        }

        _projectListWindow.processGui(_projectData);

        processMenu(pf);
        unsavedChangesOnExitPopup(pf);
    });
}

void UnTechEditor::updateProjectFile()
{
    // ::TODO add requestStopCompiling to background thread::

    _projectFile.tryWrite([&](auto& pf) {
        if (_currentEditor) {
            bool edited = _currentEditor->processPendingActions(pf);

            if (_currentEditorGui) {
                edited |= processUndoStack(_currentEditorGui, _currentEditor, pf);
            }

            if (edited) {
                _backgroundThread.markResourceInvalid(_currentEditor->itemIndex());
            }
        }

        // I am calling the closeEditor and openEditor functions inside the write mutex
        // to ensure all pending actions in AbstractEditorData and ProjectListWindow
        // have been processed before changing the editor.

        if (_projectListWindow.hasPendingActions()) {
            closeEditor(pf);
            _projectListWindow.processPendingActions(pf, _editors);

            _backgroundThread.markProjectListsInvalid();
        }

        if (_projectListWindow.selectedIndex()) {
            const auto index = *_projectListWindow.selectedIndex();
            if (_currentEditor == nullptr || _currentEditor->itemIndex() != index) {
                openEditor(pf, index);
            }
        }
        else {
            if (_currentEditor != nullptr) {
                closeEditor(pf);
            }
        }
    });
}

void BackgroundThread::run()
{
    try {
        while (threadActive) {
            pendingAction = false;

            isProcessing = true;

            {
                std::unique_lock lock(mutex);

                if (markAllResourcesInvalidFlag) {
                    projectFile.read([&](auto& pf) {
                        static_assert(std::is_const_v<std::remove_reference_t<decltype(pf)>>);

                        projectData.clearAndPopulateNamesAndDependencies(pf);
                    });

                    markResourceInvalidQueue.clear();
                    markAllResourcesInvalidFlag = false;
                }

                if (!markResourceInvalidQueue.empty()) {
                    for (auto& r : markResourceInvalidQueue) {
                        projectData.markResourceInvalid(r.type, r.index);

                        // ::TODO add separate quque for updating dependencies::
                        // ::: and add flag in EditorUndoAction to mark when dependencies changes::
                        projectFile.read([&](auto& pf) {
                            static_assert(std::is_const_v<std::remove_reference_t<decltype(pf)>>);
                            projectData.updateDependencyGraph(pf, r.type, r.index);
                        });
                    }

                    markResourceInvalidQueue.clear();
                }
            }

            if (!projectDataValid.test_and_set()) {
                // ::TODO only process resources that have changed. ::
                // ::: Use a mutex to access changed list ::

                projectFile.read([&](auto& pf) {
                    static_assert(std::is_const_v<std::remove_reference_t<decltype(pf)>>);

                    projectData.compileAll_NoEarlyExit(pf);
                });
            }

            isProcessing = false;

            std::unique_lock lock(mutex);
            cv.wait(lock, [&]() { return pendingAction || !threadActive; });
        }
    }
    catch (const std::exception&) {
    }

    isProcessing = false;
}

BackgroundThread::BackgroundThread(Project::ProjectFileMutex& pf, Project::ProjectData& pd)
    : projectFile(pf)
    , projectData(pd)
    , thread()
    , mutex()
    , cv()
    , threadActive(true)
    , pendingAction(false)
    , isProcessing(false)
    , projectDataValid(false)
    , markAllResourcesInvalidFlag(true)
    , markResourceInvalidQueue()
{
    thread = std::thread(&BackgroundThread::run, this);
}

BackgroundThread::~BackgroundThread()
{
    // Request stop thread
    {
        std::lock_guard lock(mutex);

        // ::TODO set requestStopCompiling flag::
        threadActive = false;
    }
    cv.notify_all();

    if (thread.joinable()) {
        thread.join();
    }

    assert(isProcessing == false);
}

void BackgroundThread::markProjectListsInvalid()
{
    {
        std::lock_guard lock(mutex);

        markAllResourcesInvalidFlag = true;

        projectDataValid.clear();
        pendingAction = true;
    }
    cv.notify_all();
}

void BackgroundThread::markResourceInvalid(const ItemIndex index)
{
    {
        std::lock_guard lock(mutex);

        markResourceInvalidQueue.push_back(index);

        projectDataValid.clear();
        pendingAction = true;
    }
    cv.notify_all();
}

}
