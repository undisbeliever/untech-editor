/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "untech-editor.h"
#include "abstract-editor.h"
#include "imgui-filebrowser.h"
#include "imgui.h"
#include "key-id.h"
#include "gui/style.h"
#include "gui/windows/about-popup.h"
#include "gui/windows/error-list-window.h"
#include "gui/windows/message-box.h"
#include "gui/windows/projectlist.h"
#include "models/common/imagecache.h"
#include "models/project/project.h"

namespace UnTech::Gui {

std::shared_ptr<UnTechEditor> UnTechEditor::_instance = nullptr;

UnTechEditor::UnTechEditor(std::unique_ptr<UnTech::Project::ProjectFile>&& pf, const std::filesystem::path& fn)
    : _projectFile(std::move(pf))
    , _projectData()
    , _backgroundThread(_projectFile, _projectData)
    , _filename(fn)
    , _basename(fn.filename().u8string())
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
        throw invalid_argument("filename cannot be empty");
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
        MsgBox::showMessage("Cannot Create Project", ex.what());
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

        pf->loadAllFilesIgnoringErrors();

        ImGui::setFileDialogDirectory(filename.parent_path());

        _instance = std::shared_ptr<UnTechEditor>(new UnTechEditor(std::move(pf), filename));
    }
    catch (const std::exception& ex) {
        MsgBox::showMessage("Unable to Load Project", ex.what());
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
void UnTechEditor::openEditor(const ItemIndex itemIndex)
{
    AbstractEditorData* editor = nullptr;

    auto it = std::find_if(_editors.cbegin(), _editors.cend(),
                           [&](auto& e) { return e->itemIndex() == itemIndex; });
    if (it != _editors.cend()) {
        editor = it->get();
    }

    if (editor == nullptr) {
        // Create editor
        _projectFile.read([&](auto& pf) {
            if (auto e = createEditor(itemIndex, pf)) {
                if (e->loadDataFromProject(pf)) {
                    // itemIndex is valid
                    editor = e.get();
                    _editors.push_back(std::move(e));
                }
            }
        });
    }

    if (editor != _currentEditor) {
        closeEditor();

        _currentEditor = editor;
        if (editor) {
            bool success = false;
            _projectFile.read([&](auto& pf) {
                success = editor->loadDataFromProject(pf);
            });

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

                _currentEditorGui->resetState();
            }
            else {
                _currentEditor = nullptr;
                _currentEditorGui = nullptr;
            }
        }
    }
}

// MUST ONLY be called by `updateProjectFile`
void UnTechEditor::closeEditor()
{
    if (_currentEditorGui) {
        assert(_currentEditor);

        _currentEditorGui->editorClosed();

        // AbstractEditorGui::editorClosed() may add actions to the editor.
        forceProcessEditorActions();
    }

    _currentEditor = nullptr;
    _currentEditorGui = nullptr;
}

bool UnTechEditor::saveEditor(AbstractExternalFileEditorData* editor)
{
    assert(editor);
    assert(editor->hasPendingActions() == false);

    // ::TODO is this necessary? ::
    _projectFile.read([&](auto& pf) {
        bool dataLoaded = editor->loadDataFromProject(pf);
        assert(dataLoaded);
    });

    assert(editor->filename().empty() == false);
    try {
        editor->saveFile();
        editor->markClean();
        return true;
    }
    catch (const std::exception& ex) {
        MsgBox::showMessage("Cannot Save Resource", ex.what());
        return false;
    }
}

bool UnTechEditor::saveProjectFile()
{
    assert(!_filename.empty());

    forceProcessEditorActions();

    try {
        _projectFile.read([&](auto& pf) {
            UnTech::Project::saveProjectFile(pf, _filename);
        });

        for (auto& editor : _editors) {
            if (dynamic_cast<AbstractExternalFileEditorData*>(editor.get()) == nullptr) {
                editor->markClean();
            }
        }
        _projectListWindow.markClean();
        return true;
    }
    catch (const std::exception& ex) {
        MsgBox::showMessage("Cannot Save Project", ex.what());
        return false;
    }
}

void UnTechEditor::saveCurrentEditor()
{
    if (_currentEditor) {
        forceProcessEditorActions();

        if (auto* e = dynamic_cast<AbstractExternalFileEditorData*>(_currentEditor)) {
            saveEditor(e);
        }
        else {
            saveProjectFile();
        }
    }
}

bool UnTechEditor::saveAll()
{
    forceProcessEditorActions();

    if (_currentEditor) {
        assert(_currentEditor->hasPendingActions() == false);
    }

    bool ok = true;
    bool projectFileDirty = !_projectListWindow.isClean();

    for (auto& editor : _editors) {
        if (!editor->isClean()) {
            if (auto* e = dynamic_cast<AbstractExternalFileEditorData*>(editor.get())) {
                saveEditor(e);
            }
            else {
                projectFileDirty = true;
            }
        }
    }

    if (projectFileDirty) {
        ok &= saveProjectFile();
    }

    return ok;
}

void UnTechEditor::invalidateImageCache()
{
    ImageCache::invalidateImageCache();
    _backgroundThread.markAllResourcesInvalid();
    if (_currentEditorGui) {
        _currentEditorGui->resetState();
    }
}

void UnTechEditor::processMenu()
{
    using namespace std::string_literals;

    ImGui::BeginMainMenuBar();

    if (ImGui::BeginMenu("File")) {
        if (_currentEditor) {
            if (!_currentEditor->basename().empty()) {
                auto s = "Save "s + _currentEditor->basename();
                if (ImGui::MenuItem(s.c_str())) {
                    saveCurrentEditor();
                }
            }
        }
        const auto saveProjectLabel = u8"Save "s + _basename;
        if (ImGui::MenuItem(u8Cast(saveProjectLabel), "Ctrl+S")) {
            saveProjectFile();
        }
        if (ImGui::MenuItem("Save All", "Ctrl+Shift+S")) {
            saveAll();
        }

        ImGui::Separator();

        _projectListWindow.processMenu();

        ImGui::Separator();
        if (ImGui::MenuItem("Invalidate ImageCache and Recompile Everything", "Ctrl+`")) {
            invalidateImageCache();
        }

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

        if (ImGui::MenuItem("Undo", "Ctrl+Z", false, canUndo)) {
            if (_currentEditorGui) {
                _currentEditorGui->undoClicked = true;
            }
        }
        if (ImGui::MenuItem("Redo", "Ctrl+Y / Ctrl+Shift+Z", false, canRedo)) {
            if (_currentEditorGui) {
                _currentEditorGui->redoClicked = true;
            }
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View")) {
        if (ImGui::BeginMenu("Aspect Ratio")) {
            auto apMenuItem = [](const char* name, const ZoomAspectRatio ap) {
                if (ImGui::MenuItem(name, nullptr, Style::aspectRatio == ap)) {
                    Style::setAspectRatio(ap);
                }
            };
            apMenuItem("Ntsc", ZoomAspectRatio::Ntsc);
            apMenuItem("Pal", ZoomAspectRatio::Pal);
            apMenuItem("Square", ZoomAspectRatio::Square);

            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (_currentEditorGui) {
            _currentEditorGui->viewMenu();
        }

        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

void UnTechEditor::processKeyboardShortcuts()
{
    const auto& io = ImGui::GetIO();

    if (io.KeyCtrl) {
        if (ImGui::IsKeyPressed(KeyId_S, false)) {
            if (io.KeyShift) {
                saveAll();
            }
            else {
                saveCurrentEditor();
            }
        }

        if (_currentEditorGui && ImGui::IsAnyItemActive() == false) {
            if (ImGui::IsKeyPressed(KeyId_Z, false)) {
                if (io.KeyShift) {
                    _currentEditorGui->redoClicked = true;
                }
                else {
                    _currentEditorGui->undoClicked = true;
                }
            }
            if (ImGui::IsKeyPressed(KeyId_Y, false)) {
                _currentEditorGui->redoClicked = true;
            }
        }

        if (ImGui::IsKeyPressed(KeyId_Grave, false)) {
            invalidateImageCache();
        }
    }
}

void UnTechEditor::requestExitEditor()
{
    const auto parentPath = _filename.parent_path();

    std::vector<std::u8string> files;
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

void UnTechEditor::unsavedChangesOnExitPopup()
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
                _editorExited = saveAll();
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

        const ImVec2 unsavedListSize(0, ImGui::GetTextLineHeightWithSpacing() * 4.25f);
        ImGui::SetNextItemWidth(-style.IndentSpacing);
        if (ImGui::BeginListBox("##UnsavedFiles", unsavedListSize)) {
            for (const auto& f : _unsavedFilesList) {
                ImGui::TextUnformatted(u8Cast(f));
            }
            ImGui::EndListBox();
        }

        ImGui::Unindent();

        ImGui::EndPopup();
    }
}

void UnTechEditor::processGui()
{
    if (_currentEditorGui) {
        assert(_currentEditor);

        _projectFile.read([&](auto& pf) {
            _currentEditorGui->processGui(pf, _projectData);
        });

        processErrorListWindow(_projectData, _currentEditor);
    }

    _projectListWindow.processGui(_projectData);

    processMenu();
    processKeyboardShortcuts();
    unsavedChangesOnExitPopup();

    if (_currentEditor) {
        _currentEditor->processEditorActions(_currentEditorGui);
        _currentEditor->updateSelection();
    }
}

void UnTechEditor::updateProjectFile()
{
    if (_currentEditor) {
        bool edited = false;

        // ::TODO add requestStopCompiling to background thread::

        _projectFile.tryWrite([&](auto& pf) {
            edited = _currentEditor->processPendingProjectActions(pf);

            if (_currentEditorGui) {
                edited |= processUndoStack(_currentEditorGui, _currentEditor, pf);
            }
        });

        if (edited) {
            _backgroundThread.markResourceInvalid(_currentEditor->itemIndex());
        }
    }

    if (_projectListWindow.hasPendingActions()) {
        closeEditor();

        _projectFile.write([&](auto& pf) {
            _projectListWindow.processPendingActions(pf, _editors);
        });

        _backgroundThread.markAllResourcesInvalid();
    }

    if (_projectListWindow.selectedIndex()) {
        const auto index = *_projectListWindow.selectedIndex();
        if (_currentEditor == nullptr || _currentEditor->itemIndex() != index) {
            openEditor(index);
        }
    }
    else {
        if (_currentEditor != nullptr) {
            closeEditor();
        }
    }
}

void UnTechEditor::forceProcessEditorActions()
{
    // ::TODO add requestStopCompiling to background thread::

    if (_currentEditor) {
        bool edited = false;

        _currentEditor->processEditorActions(_currentEditorGui);

        _projectFile.write([&](auto& pf) {
            edited = _currentEditor->processPendingProjectActions(pf);
        });

        if (edited) {
            _backgroundThread.markResourceInvalid(_currentEditor->itemIndex());
        }

        _currentEditor->updateSelection();
    }
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
                projectFile.read([&](auto& pf) {
                    static_assert(std::is_const_v<std::remove_reference_t<decltype(pf)>>);

                    projectData.compileAll_NoEarlyExit(pf);

                    processEntityGraphics(pf, projectData);
                });
            }

            isProcessing = false;

            std::unique_lock lock(mutex);
            cv.wait(lock, [&]() { return pendingAction || !threadActive; });
        }
    }
    catch (const std::exception& ex) {
        MsgBox::showMessage("An exception occurred when compiling a resource",
                            stringBuilder(ex.what(), "\n\n\nThis should not happen."
                                                     "\n\nThe resource compiler is now disabled."));

        projectData.markAllResourcesInvalid();
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

void BackgroundThread::markAllResourcesInvalid()
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
