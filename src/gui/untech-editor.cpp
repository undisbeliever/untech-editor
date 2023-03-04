/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "untech-editor.h"
#include "abstract-editor.h"
#include "imgui-filebrowser.h"
#include "imgui.h"
#include "gui/style.h"
#include "gui/windows/about-popup.h"
#include "gui/windows/error-list-window.h"
#include "gui/windows/message-box.h"
#include "gui/windows/projectlist.h"
#include "models/common/imagecache.h"
#include "models/project/project.h"

namespace UnTech::Gui {

std::shared_ptr<UnTechEditor> UnTechEditor::_instance = nullptr;

UnTechEditor::UnTechEditor(std::unique_ptr<UnTech::Project::ProjectFile> pf, const std::filesystem::path& fn)
    : _backgroundThread(std::move(pf))
    , _filename(fn)
    , _basename(fn.filename().u8string())
    , _editorGuis(createEditorGuis())
    , _editors()
    , _currentEditor(nullptr)
    , _currentEditorGui(nullptr)
    , _lastCompileId(UINT64_MAX)
    , _projectListWindow()
    , _projectListSidebar{ 280, 150, 500 }
    , _showProjectListSidebar(true)
    , _openUnsavedChangesOnExitPopup(false)
    , _editorExited(false)
    , _unsavedFilesList()
{
    if (_filename.empty()) {
        throw invalid_argument(u8"filename cannot be empty");
    }
}

void UnTechEditor::newProject(const std::filesystem::path& filename)
{
    if (_instance) {
        return;
    }

    if (filename.empty()) {
        return;
    }

    try {
        // no need for an std::error_code, we are inside a try/catch block
        const auto absFilename = std::filesystem::absolute(filename).lexically_normal();

        if (std::filesystem::exists(absFilename)) {
            loadProject(absFilename);
            return;
        }

        auto pf = std::make_unique<UnTech::Project::ProjectFile>();
        UnTech::Project::saveProjectFile(*pf, absFilename);

        ImGui::setFileDialogDirectory(absFilename.parent_path());

        _instance = std::shared_ptr<UnTechEditor>(new UnTechEditor(std::move(pf), absFilename));
    }
    catch (const std::exception& ex) {
        MsgBox::showMessage(u8"Cannot Create Project", ex.what());
    }
}

void UnTechEditor::loadProject(const std::filesystem::path& filename)
{
    if (_instance) {
        return;
    }

    if (filename.empty()) {
        return;
    }

    try {
        // no need for an std::error_code, we are inside a try/catch block
        const auto absFilename = std::filesystem::absolute(filename).lexically_normal();

        auto pf = UnTech::Project::loadProjectFile(absFilename);

        pf->loadAllFilesIgnoringErrors();

        ImGui::setFileDialogDirectory(absFilename.parent_path());

        _instance = std::shared_ptr<UnTechEditor>(new UnTechEditor(std::move(pf), absFilename));
    }
    catch (const std::exception& ex) {
        MsgBox::showMessage(u8"Unable to Load Project", ex.what());
    }
}

void UnTechEditor::closeProject()
{
    if (_instance) {
        _instance.reset();
    }
}

// MUST ONLY be called by `updateProjectFile`
void UnTechEditor::openEditor(const ItemIndex itemIndex)
{
    std::shared_ptr<AbstractEditorData> editor = nullptr;

    auto it = std::find_if(_editors.cbegin(), _editors.cend(),
                           [&](const auto& e) { return e->itemIndex() == itemIndex; });
    if (it != _editors.cend()) {
        editor = *it;
    }

    if (editor == nullptr) {
        // Create editor
        _backgroundThread.read_pf([&](const auto& pf) {
            if (auto e = createEditor(itemIndex, pf)) {
                if (e->loadDataFromProject(pf)) {
                    // itemIndex is valid
                    editor = e;
                    _editors.push_back(e);
                }
            }
        });
    }

    if (editor != _currentEditor) {
        closeEditor();

        _currentEditor = editor;
        if (editor) {
            bool success = false;
            _backgroundThread.read_pf([&](const auto& pf) {
                success = editor->loadDataFromProject(pf);
            });

            if (success) {
                unsigned counter = 0;
                for (auto& eg : _editorGuis) {
                    if (eg->setEditorData(_currentEditor)) {
                        _currentEditorGui = eg;
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

    // Ensure `AbstractEditorGui::resourceCompiled` is called on the next frame.
    _lastCompileId = UINT64_MAX;
}

bool UnTechEditor::saveEditor(AbstractExternalFileEditorData* editor)
{
    assert(editor);

    // Silence a [assertWithSideEffect] cppcheck warning
    const auto& const_undoStack = _currentEditor->undoStack();
    assert(const_undoStack.hasPendingActions() == false);

    // ::TODO is this necessary? ::
    _backgroundThread.read_pf([&](const auto& pf) {
        bool dataLoaded = editor->loadDataFromProject(pf);
        assert(dataLoaded);
    });

    assert(editor->filename().empty() == false);
    try {
        editor->saveFile();
        editor->undoStack().markClean();
        return true;
    }
    catch (const std::exception& ex) {
        MsgBox::showMessage(u8"Cannot Save Resource", ex.what());
        return false;
    }
}

bool UnTechEditor::saveProjectFile()
{
    assert(!_filename.empty());

    forceProcessEditorActions();

    try {
        _backgroundThread.read_pf([&](const auto& pf) {
            UnTech::Project::saveProjectFile(pf, _filename);
        });

        for (auto& editor : _editors) {
            if (dynamic_cast<AbstractExternalFileEditorData*>(editor.get().get()) == nullptr) {
                editor->undoStack().markClean();
            }
        }
        _projectListWindow.markClean();
        return true;
    }
    catch (const std::exception& ex) {
        MsgBox::showMessage(u8"Cannot Save Project", ex.what());
        return false;
    }
}

void UnTechEditor::saveCurrentEditor()
{
    if (_currentEditor) {
        forceProcessEditorActions();

        if (auto* e = dynamic_cast<AbstractExternalFileEditorData*>(_currentEditor.get())) {
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
        // Silence a [assertWithSideEffect] cppcheck warning
        const auto& undoStack = _currentEditor->undoStack();
        assert(undoStack.hasPendingActions() == false);
    }

    bool ok = true;
    bool projectFileDirty = !_projectListWindow.isClean();

    for (auto& editor : _editors) {
        if (!editor->undoStack().isClean()) {
            if (auto* e = dynamic_cast<AbstractExternalFileEditorData*>(editor.get().get())) {
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
    _backgroundThread.markResourceListMovedOrResized();
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
                auto s = u8"Save "s + _currentEditor->basename();
                if (ImGui::MenuItem(u8Cast(s))) {
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
        const bool canUndo = _currentEditor && _currentEditor->undoStack().canUndo();
        const bool canRedo = _currentEditor && _currentEditor->undoStack().canRedo();

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

        ImGui::MenuItem("Project List Sidebar", "`", &_showProjectListSidebar);

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

    if (io.KeyMods == ImGuiModFlags_Ctrl) {
        if (ImGui::IsKeyPressed(ImGuiKey_S, false)) {
            if (io.KeyShift) {
                saveAll();
            }
            else {
                saveCurrentEditor();
            }
        }

        if (_currentEditorGui && ImGui::IsAnyItemActive() == false) {
            if (ImGui::IsKeyPressed(ImGuiKey_Z, false)) {
                if (io.KeyShift) {
                    _currentEditorGui->redoClicked = true;
                }
                else {
                    _currentEditorGui->undoClicked = true;
                }
            }
            if (ImGui::IsKeyPressed(ImGuiKey_Y, false)) {
                _currentEditorGui->redoClicked = true;
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent, false)) {
            invalidateImageCache();
        }
    }

    if (io.KeyMods == ImGuiModFlags_None) {
        if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent, false)) {
            _showProjectListSidebar = !_showProjectListSidebar;
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
        if (!e->undoStack().isClean()) {
            if (auto* ee = dynamic_cast<AbstractExternalFileEditorData*>(e.get().get())) {
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
            ImGui::TextUnformatted(u8"There is one unsaved file.");
            ImGui::NewLine();
            ImGui::TextUnformatted(u8"Do you wish to save?"s);
        }
        else {
            ImGui::Text("There are %d unsaved files.", int(_unsavedFilesList.size()));
            ImGui::NewLine();
            ImGui::TextUnformatted(u8"Do you wish to save them all?");
        }

        ImGui::TextUnformatted(u8"\n(Unsaved changes will be lost if you discard them)"s);

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

void UnTechEditor::fullscreenBackgroundWindow()
{
    constexpr auto windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar
                                 | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoSavedSettings;

    const auto* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    if (ImGui::Begin("##BgWindow", nullptr, windowFlags)) {
        ImVec2 editorSize{ 0, 0 };

        if (_showProjectListSidebar) {
            auto s = splitterSidebarLeft("plSplitter", &_projectListSidebar);

            ImGui::BeginChild("ProjectList", s.first, false);
            _projectListWindow.processGui(_backgroundThread.compilerStatus());
            ImGui::EndChild();

            editorSize = s.second;
        }

        if (_currentEditorGui) {
            assert(_currentEditor);

            ImGui::SameLine();
            ImGui::BeginChild(_currentEditorGui->childWindowStrId, editorSize, false);
            _backgroundThread.read_pf([&](const auto& pf) {
                _currentEditorGui->processGui(pf, _backgroundThread.projectData());
            });
            ImGui::EndChild();
        }
    }
    ImGui::End();
}

void UnTechEditor::processGui()
{
    fullscreenBackgroundWindow();

    if (_currentEditor) {
        const auto& itemIndex = _currentEditor->itemIndex();

        _backgroundThread.compilerStatus().readResourceState(
            itemIndex.type, itemIndex.index,
            [&](const auto& rs) {
                if (rs.compileId != _lastCompileId) {
                    _lastCompileId = rs.compileId;
                    _currentEditorGui->resourceCompiled(rs.errorList);
                }

                processErrorListWindow(_currentEditor.get(), rs.state, rs.errorList);
            });

        _backgroundThread.read_pf([&](const auto& pf) {
            _currentEditorGui->processExtraWindows(pf, _backgroundThread.projectData());
        });
    }

    processMenu();
    processKeyboardShortcuts();
    unsavedChangesOnExitPopup();

    if (_currentEditor) {
        _currentEditor->undoStack().processEditorActions(_currentEditorGui.get());
        _currentEditor->updateSelection();
    }
}

void UnTechEditor::updateProjectFile()
{
    if (_currentEditor) {
        assert(_currentEditorGui);

        bool edited = false;

        if (_currentEditor->undoStack().hasPendingActions()
            || _currentEditorGui->hasPendingUndoRedo()) {

            _backgroundThread.tryWrite_pf([&](auto& pf) {
                edited |= processUndoStack(_currentEditor.get(), _currentEditorGui.get(), pf);
            });
        }

        if (edited) {
            _backgroundThread.markResourceUnchecked(_currentEditor->itemIndex());
        }
    }

    if (_projectListWindow.hasPendingActions()) {
        closeEditor();

        _backgroundThread.write_pf([&](auto& pf) {
            _projectListWindow.processPendingActions(pf, _editors);
        });

        _backgroundThread.markResourceListMovedOrResized();
    }

    const auto selectedIndex = _projectListWindow.selectedIndex();
    if (selectedIndex) {
        if (_currentEditor == nullptr || _currentEditor->itemIndex() != selectedIndex.value()) {
            openEditor(selectedIndex.value());
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

        _backgroundThread.write_pf([&](auto& pf) {
            edited |= processUndoStack(_currentEditor.get(), _currentEditorGui.get(), pf);
        });

        if (edited) {
            _backgroundThread.markResourceUnchecked(_currentEditor->itemIndex());
        }

        _currentEditor->updateSelection();
    }
}

}
