/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "untech-editor.h"
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
    , _selectedItem()
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

void UnTechEditor::openEditor(EditorType type, unsigned item)
{
    _selectedItem = ItemIndex(type, item);

    ImGui::LogText("Open Editor %d %d", unsigned(type), item);
}

void UnTechEditor::closeEditor()
{
    _selectedItem = std::nullopt;
}

void UnTechEditor::processGui()
{
    ProjectListWindow::processGui(*this);
}

void UnTechEditor::updateProjectFile()
{
}

}
