/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "error-list-window.h"
#include "gui/abstract-editor.h"
#include "gui/imgui.h"
#include "gui/style.h"

namespace UnTech::Gui {

using RS = Project::ResourceState;

void processErrorListWindow(gsl::not_null<AbstractEditorData*> editorData, const Project::ResourceState resourceState, const ErrorList& errors)
{
    if (errors.empty() && resourceState != RS::DependencyError) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(600, 200), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Error List")) {

        if (resourceState == RS::DependencyError) {
            ImGui::PushStyleColor(ImGuiCol_Text, Style::failColor);
            ImGui::Bullet();
            ImGui::PopStyleColor();

            ImGui::SameLine();
            ImGui::TextUnformatted(u8"Dependency Error");
        }

        for (const auto& item : errors.list()) {
            if (!item->isWarning) {
                ImGui::PushStyleColor(ImGuiCol_Text, Style::failColor);
            }
            else {
                ImGui::PushStyleColor(ImGuiCol_Text, Style::warningColor);
            }

            ImGui::Bullet();
            ImGui::PopStyleColor();

            ImGui::SameLine();
            ImGui::TextUnformatted(item->message);

            if (ImGui::IsItemClicked()) {
                if (ImGui::IsMouseDoubleClicked(0)) {
                    editorData->errorDoubleClicked(item.get());
                }
            }
        }
    }

    ImGui::End();
}

}
