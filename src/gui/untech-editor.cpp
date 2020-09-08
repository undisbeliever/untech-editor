/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "untech-editor.h"
#include "abstract-editor.h"
#include "enums.h"
#include "imgui.h"
#include "gui/windows/about-popup.h"
#include "gui/windows/message-box.h"
#include "models/project/project.h"
#include "windows/projectlist.h"

namespace UnTech::Gui {

std::shared_ptr<UnTechEditor> UnTechEditor::_instance = nullptr;

UnTechEditor::UnTechEditor(std::unique_ptr<UnTech::Project::ProjectFile>&& pf, const std::filesystem::__cxx11::path& fn)
    : _projectFile(std::move(pf))
    , _filename(fn)
    , _basename(fn.filename())
    , _editors()
    , _currentEditor(nullptr)
{
    assert(_projectFile != nullptr);
    assert(!_filename.empty());
}

void UnTechEditor::newProject(const std::filesystem::path& filename)
{
    if (_instance) {
        return;
    }

    if (filename.empty()) {
        return;
    }

    if (std::filesystem::exists(filename)) {
        loadProject(filename);
        return;
    }

    try {
        auto pf = std::make_unique<UnTech::Project::ProjectFile>();
        UnTech::Project::saveProjectFile(*pf, filename);

        _instance = std::shared_ptr<UnTechEditor>(new UnTechEditor(std::move(pf), filename));
    }
    catch (const std::exception& ex) {
        MessageBox::showMessage("Cannot Create Project", ex.what());
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
        auto pf = UnTech::Project::loadProjectFile(filename);

        // ::TODO move into background thread::
        pf->loadAllFiles();

        _instance = std::shared_ptr<UnTechEditor>(new UnTechEditor(std::move(pf), filename));
    }
    catch (const std::exception& ex) {
        MessageBox::showMessage("Unable to Load Project", ex.what());
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

void UnTechEditor::openEditor(const ItemIndex itemIndex)
{
    AbstractEditor* editor = nullptr;

    auto it = std::find_if(_editors.cbegin(), _editors.cend(),
                           [&](auto& e) { return e->itemIndex() == itemIndex; });
    if (it != _editors.cend()) {
        editor = it->get();
    }

    if (editor == nullptr) {
        if (auto e = createEditor(itemIndex, *_projectFile)) {
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
        // Discard any uncommitted data
        _currentEditor->loadDataFromProject(*_projectFile);
        _currentEditor->editorClosed();
        _currentEditor = nullptr;
    }
}

void UnTechEditor::saveProjectFile()
{
    assert(_projectFile);
    assert(!_filename.empty());

    try {
        UnTech::Project::saveProjectFile(*_projectFile, _filename);

        for (auto& e : _editors) {
            if (dynamic_cast<AbstractExternalFileEditor*>(_currentEditor) == nullptr) {
                e->markClean();
            }
        }
    }
    catch (const std::exception& ex) {
        MessageBox::showMessage("Cannot Save Project", ex.what());
    }
}

void UnTechEditor::saveEditor(AbstractEditor* editor)
{
    if (auto e = dynamic_cast<AbstractExternalFileEditor*>(editor)) {
        bool dataLoaded = e->loadDataFromProject(*_projectFile);
        assert(dataLoaded);
        assert(e->filename().empty() == false);
        try {
            e->saveFile();
            e->markClean();
        }
        catch (const std::exception& ex) {
            MessageBox::showMessage("Cannot Save Resource", ex.what());
        }
    }
    else {
        saveProjectFile();
    }
}

void UnTechEditor::saveAll()
{
    for (auto& e : _editors) {
        if (!e->isClean()) {
            saveEditor(e.get());
        }
    }
}

void UnTechEditor::processMenu()
{
    using namespace std::string_literals;

    ImGui::BeginMainMenuBar();

    if (ImGui::BeginMenu("File")) {
        // ::TODO add Ctrl+S shortcut::
        if (_currentEditor) {
            if (!_currentEditor->basename().empty()) {
                auto s = "Save "s + _currentEditor->basename();
                if (ImGui::MenuItem(s.c_str())) {
                    saveEditor(_currentEditor);
                }
            }
        }
        const auto saveProjectLabel = "Save "s + _basename;
        if (ImGui::MenuItem(saveProjectLabel.c_str())) {
            saveProjectFile();
        }
        if (ImGui::MenuItem("Save All")) {
            saveAll();
        }

        ImGui::Separator();

        _projectListWindow.processMenu();

        ImGui::Separator();

        if (ImGui::MenuItem("About UnTech Editor")) {
            AboutPopup::openPopup();
        }

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

void UnTechEditor::processGui()
{
    const Project::ProjectFile& projectFile = *_projectFile;

    if (_currentEditor) {
        _currentEditor->processGui(projectFile);
        _currentEditor->updateSelection();
    }

    _projectListWindow.processGui(projectFile);

    processMenu();
}

void UnTechEditor::updateProjectFile()
{
    if (_currentEditor) {
        _currentEditor->processPendingActions(*_projectFile);
    }

    if (_projectListWindow.hasPendingActions()) {
        closeEditor();
    }
    _projectListWindow.processPendingActions(*_projectFile, _editors);

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

}
