/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstract-metasprite-editor.h"
#include "gui/style.h"
#include "models/common/stringbuilder.h"

namespace UnTech::Gui {

// ::TODO add to View Menu::
bool AbstractMetaSpriteEditorGui::showFrameObjects = true;
bool AbstractMetaSpriteEditorGui::showTileHitbox = true;
bool AbstractMetaSpriteEditorGui::showEntityHitboxes = true;
bool AbstractMetaSpriteEditorGui::showActionPoints = true;

void AbstractMetaSpriteEditorGui::editorDataChanged()
{
    invalidateExportOrderTree();
}

void AbstractMetaSpriteEditorGui::showLayerButtons() const
{
    ImGui::ToggledButtonWithTooltip("TH##showTH", &showTileHitbox, "Show Tile Hitbox");
    ImGui::SameLine();
    ImGui::ToggledButtonWithTooltip("FO##showFO", &showFrameObjects, "Show Frame Objects");
    ImGui::SameLine();
    ImGui::ToggledButtonWithTooltip("AP##showAP", &showActionPoints, "Show Action Points");
    ImGui::SameLine();
    ImGui::ToggledButtonWithTooltip("EH##showEH", &showEntityHitboxes, "Show Entity Hitboxes");
}

static void exportOrderTree(const std::vector<AbstractMetaSpriteEditorGui::ExportOrderTree>& tree,
                            AbstractMetaSpriteEditorGui* gui,
                            std::function<void(AbstractMetaSpriteEditorGui*, const idstring&)> addFunction,
                            const std::string& addSuffix)
{
    static unsigned contextMenuIndex = INT_MAX;

    for (unsigned i = 0; i < tree.size(); i++) {
        auto& eo = tree.at(i);

        // ::TODO replace with icons::
        ImGui::PushStyleColor(ImGuiCol_Text, Style::successFailColor(eo.valid));
        ImGui::TextUnformatted(eo.valid ? u8"·" : u8"X");
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::TextUnformatted(eo.name.str());

        if (!eo.valid) {
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
                contextMenuIndex = i;
                ImGui::OpenPopup("context");
            }
        }
    }

    if (ImGui::BeginPopup("context")) {
        if (contextMenuIndex < tree.size()) {
            auto& eo = tree.at(contextMenuIndex);

            auto menuItem = [&](const idstring& name) {
                const std::string text = stringBuilder("Add ", name, addSuffix);
                if (ImGui::MenuItem(text.c_str())) {
                    addFunction(gui, name);
                }
            };
            menuItem(eo.name);

            for (unsigned i = 0; i < eo.alternatives.size(); i++) {
                const auto& alt = eo.alternatives.at(i);

                ImGui::PushID(i);
                menuItem(alt);
                ImGui::PopID();
            }
        }
        ImGui::EndPopup();
    }
}

void AbstractMetaSpriteEditorGui::exportOrderWindow(const char* windowLabel)
{
    using namespace std::string_literals;

    if (ImGui::Begin(windowLabel)) {
        ImGui::SetWindowSize(ImVec2(325, 650), ImGuiCond_FirstUseEver);

        if (!_eoStillFrames.empty() || !_eoAnimations.empty()) {
            if (ImGui::TreeNodeEx("Still Frames", ImGuiTreeNodeFlags_DefaultOpen)) {
                exportOrderTree(_eoStillFrames, this, &AbstractMetaSpriteEditorGui::addFrame, " Frame"s);
                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("Animations", ImGuiTreeNodeFlags_DefaultOpen)) {
                exportOrderTree(_eoAnimations, this, &AbstractMetaSpriteEditorGui::addAnimation, " Animation"s);
                ImGui::TreePop();
            }
        }
    }
    ImGui::End();
}

}
