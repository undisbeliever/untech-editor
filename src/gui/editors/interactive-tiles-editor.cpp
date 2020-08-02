/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "interactive-tiles-editor.h"
#include "gui/imgui.h"
#include "gui/list-helpers.h"

namespace UnTech::Gui {

InteractiveTilesEditor::InteractiveTilesEditor(ItemIndex itemIndex)
    : AbstractEditor(itemIndex)
{
}

bool InteractiveTilesEditor::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    _interactiveTiles = projectFile.interactiveTiles;

    return true;
}

void InteractiveTilesEditor::commitPendingChanges(Project::ProjectFile& projectFile)
{
    projectFile.interactiveTiles = _interactiveTiles;
}

void InteractiveTilesEditor::editorOpened()
{
}

void InteractiveTilesEditor::editorClosed()
{
}

void InteractiveTilesEditor::processGui(const Project::ProjectFile&)
{
    if (ImGui::Begin("Interactive Tiles")) {
        ImGui::SetWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);

        ListButtons(&_sel, &_interactiveTiles.functionTables, _interactiveTiles.MAX_PROJECT_FUNCTION_TABLES);

        ImGui::BeginChild("scroll");

        ImGui::Columns(4);
        ImGui::SetColumnWidth(0, 40);

        ImGui::Separator();
        ImGui::NextColumn();
        ImGui::Text("Name");
        ImGui::NextColumn();
        ImGui::Text("Symbol");
        ImGui::NextColumn();
        ImGui::Text("Symbol Color");
        ImGui::NextColumn();
        ImGui::Separator();

        unsigned engineId = 0;
        for (const auto& ft : _interactiveTiles.FIXED_FUNCTION_TABLES) {
            ImGui::Text("%d", engineId++);
            ImGui::NextColumn();
            ImGui::TextUnformatted(ft.name);
            ImGui::NextColumn();

            ImGui::TextUnformatted(ft.symbol);
            ImGui::NextColumn();

            ImGui::Text("%06X", ft.symbolColor.rgbHex());
            // ::TODO add color square::
            ImGui::NextColumn();
        }

        ImGui::Separator();

        for (unsigned i = 0; i < _interactiveTiles.functionTables.size(); i++) {
            auto& ft = _interactiveTiles.functionTables.at(i);

            bool edited = false;

            ImGui::PushID(i);

            const std::string selLabel = std::to_string(engineId++);
            ImGui::Selectable(selLabel.c_str(), &_sel, i);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputIdstring("##Name", &ft.name);
            edited |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputText("##Symbol", &ft.symbol);
            edited |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputRgb("##Color", &ft.symbolColor);
            edited |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::NextColumn();

            if (edited) {
                ImGui::LogText("Edited Interactive Tiles");
                this->pendingChanges = true;
            }

            ImGui::PopID();

            UpdateSelection(&_sel);
        }

        ImGui::Columns(1);
        ImGui::Separator();

        ImGui::EndChild();
    }
    ImGui::End();
}

}
