/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "error-list-window.h"
#include "gui/abstract-editor.h"
#include "gui/imgui.h"
#include "gui/style.h"
#include "models/project/project-data.h"

namespace UnTech::Gui {

void processErrorListWindow(const Project::ResourceStatus& status)
{
    if (status.errorList.empty()) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(600, 200), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Error List")) {

        for (const auto& item : status.errorList.list()) {
            if (!item.isWarning) {
                ImGui::PushStyleColor(ImGuiCol_Text, Style::failColor);
            }
            else {
                ImGui::PushStyleColor(ImGuiCol_Text, Style::warningColor);
            }

            // ::TODO add double click on error to select bad item::

            ImGui::Bullet();
            ImGui::PopStyleColor();

            ImGui::SameLine();
            ImGui::TextUnformatted(item.message);
        }
    }

    ImGui::End();
}

void processErrorListWindow(const Project::ProjectData& projectData, AbstractEditorData* editorData)
{
    if (editorData == nullptr) {
        return;
    }

    const auto itemIndex = editorData->itemIndex();

    const auto& list = projectData.resourceListStatus(itemIndex.type);

    list.readResourceListState([&](auto& state, auto& resources) {
        static_assert(std::is_const_v<std::remove_reference_t<decltype(state)>>);
        static_assert(std::is_const_v<std::remove_reference_t<decltype(resources)>>);

        if (itemIndex.index < resources.size()) {
            processErrorListWindow(resources.at(itemIndex.index));
        }
    });
}

}
