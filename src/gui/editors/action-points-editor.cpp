/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "action-points-editor.h"
#include "gui/imgui.h"
#include "gui/list-helpers.h"

namespace UnTech::Gui {

ActionPointsEditor::ActionPointsEditor(ItemIndex itemIndex)
    : AbstractEditor(itemIndex)
    , _actionPoints()
    , _sel()
{
}

bool ActionPointsEditor::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    _actionPoints = projectFile.actionPointFunctions;

    return true;
}

void ActionPointsEditor::commitPendingChanges(Project::ProjectFile& projectFile)
{
    projectFile.actionPointFunctions = _actionPoints;
}

void ActionPointsEditor::editorOpened()
{
}

void ActionPointsEditor::editorClosed()
{
}

void ActionPointsEditor::actionPointsWindow()
{
    if (ImGui::Begin("Action Points")) {
        ImGui::SetWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);

        ListButtons(&_sel, &_actionPoints, MetaSprite::MAX_ACTION_POINT_FUNCTIONS);

        ImGui::BeginChild("Scroll");

        ImGui::Columns(3);
        ImGui::SetColumnWidth(0, 40);

        ImGui::Separator();
        ImGui::NextColumn();
        ImGui::Text("Name");
        ImGui::NextColumn();
        ImGui::Text("Manually Invoked");
        ImGui::NextColumn();
        ImGui::Separator();

        for (unsigned i = 0; i < _actionPoints.size(); i++) {
            auto& ap = _actionPoints.at(i);

            bool edited = false;

            ImGui::PushID(i);

            ImGui::Selectable(&_sel, i);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputIdstring("##Name", &ap.name);
            edited |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            edited |= ImGui::Checkbox("##ManuallyInvoked", &ap.manuallyInvoked);
            ImGui::NextColumn();

            if (edited) {
                ImGui::LogText("Edited Interactive Tiles");
                this->pendingChanges = true;
            }

            ImGui::PopID();
        }

        ImGui::Columns(1);
        ImGui::Separator();

        ImGui::EndChild();
    }
    ImGui::End();
}

void ActionPointsEditor::processGui(const Project::ProjectFile&)
{
    actionPointsWindow();

    UpdateSelection(&_sel);
}

}
