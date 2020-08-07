/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "untech-editor.h"
#include "abstract-editor.h"
#include "enums.h"
#include "imgui.h"
#include "models/project/project.h"
#include "windows/projectlist.h"

namespace UnTech::Gui {

std::shared_ptr<UnTechEditor> UnTechEditor::_instance = nullptr;

UnTechEditor::UnTechEditor()
    : UnTechEditor(std::make_unique<UnTech::Project::ProjectFile>())
{
}

UnTechEditor::UnTechEditor(std::unique_ptr<UnTech::Project::ProjectFile>&& pf)
    : _projectFile(std::move(pf))
    , _editors()
    , _currentEditor(nullptr)
{
    assert(_projectFile != nullptr);
}

void UnTechEditor::newProject()
{
    _instance = std::shared_ptr<UnTechEditor>(new UnTechEditor());
}

void UnTechEditor::loadProject(std::filesystem::path filename)
{
    try {
        auto pf = UnTech::Project::loadProjectFile(filename);

        // ::TODO move into background thread::
        pf->loadAllFiles();

        _instance = std::shared_ptr<UnTechEditor>(new UnTechEditor(std::move(pf)));
    }
    catch (const std::exception& ex) {
        // ::TODO create message window
        ImGui::LogText("Unable to load file: %s", ex.what());
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

void UnTechEditor::openEditor(EditorType type, unsigned item)
{
    const ItemIndex itemIndex{ type, item };

    AbstractEditor* editor = nullptr;

    auto it = std::find_if(_editors.cbegin(), _editors.cend(),
                           [&](auto& e) { return e->itemIndex() == itemIndex; });
    if (it != _editors.cend()) {
        editor = it->get();
    }

    if (editor == nullptr) {
        if (auto e = createEditor(itemIndex)) {
            if (e->loadDataFromProject(*_projectFile)) {
                // itemIndex is valid
                editor = e.get();
                _editors.push_back(std::move(e));
            }
        }
    }

    if (editor != _currentEditor) {
        closeEditor();
        _currentEditor = editor;
        if (editor) {
            editor->loadDataFromProject(*_projectFile);
            editor->editorOpened();
        }
    }
}

void UnTechEditor::closeEditor()
{
    if (_currentEditor) {
        // ::TODO force update if I add multi-threading::
        updateProjectFile();

        // Discard any uncommitted data
        _currentEditor->loadDataFromProject(*_projectFile);

        _currentEditor->editorClosed();

        _currentEditor = nullptr;
    }
}

void UnTechEditor::processGui()
{
    ProjectListWindow::processGui(*this);

    if (_currentEditor) {
        _currentEditor->processGui(*_projectFile);
        _currentEditor->updateSelection();
    }

    processMenu();
}

void UnTechEditor::processMenu()
{
    ImGui::BeginMainMenuBar();

    if (ImGui::BeginMenu("File")) {
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit")) {
        const bool canUndo = _currentEditor && _currentEditor->canUndo();
        const bool canRedo = _currentEditor && _currentEditor->canRedo();

        if (ImGui::MenuItem("Undo", nullptr, false, canUndo)) {
            if (_currentEditor) {
                _currentEditor->undo(*_projectFile);
            }
        }
        if (ImGui::MenuItem("Redo", nullptr, false, canRedo)) {
            if (_currentEditor) {
                _currentEditor->redo(*_projectFile);
            }
        }

        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

void UnTechEditor::updateProjectFile()
{
    if (_currentEditor) {
        _currentEditor->processPendingActions(*_projectFile);
    }
}

}
