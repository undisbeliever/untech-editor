/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstract-metasprite-editor.h"
#include "gui/style.h"
#include "models/common/iterators.h"
#include "models/common/stringbuilder.h"

namespace UnTech::Gui {

bool AbstractMetaSpriteEditorGui::showTileHitbox = true;
bool AbstractMetaSpriteEditorGui::showShield = true;
bool AbstractMetaSpriteEditorGui::showHitbox = true;
bool AbstractMetaSpriteEditorGui::showHurtbox = true;
bool AbstractMetaSpriteEditorGui::showFrameObjects = true;
bool AbstractMetaSpriteEditorGui::showActionPoints = true;

void AbstractMetaSpriteEditorGui::resetState()
{
    _exportOrderValid = false;
}

void AbstractMetaSpriteEditorGui::viewMenu()
{
    ImGui::MenuItem("Show Tile Hitbox", nullptr, &showTileHitbox);
    ImGui::MenuItem("Show Shield Box", nullptr, &showShield);
    ImGui::MenuItem("Show Hitbox", nullptr, &showHitbox);
    ImGui::MenuItem("Show Hurtbox", nullptr, &showHurtbox);
    ImGui::MenuItem("Show Frame Objects", nullptr, &showFrameObjects);
    ImGui::MenuItem("Show Action Points", nullptr, &showActionPoints);
}

void AbstractMetaSpriteEditorGui::showLayerButtons() const
{
    ImGui::ToggledButtonWithTooltip("TH##showTH", &showTileHitbox, "Show Tile Hitbox");
    ImGui::SameLine();
    ImGui::ToggledButtonWithTooltip("SB##showSB", &showShield, "Show Shield Box");
    ImGui::SameLine();
    ImGui::ToggledButtonWithTooltip("HB##showHB", &showHitbox, "Show Hitbox");
    ImGui::SameLine();
    ImGui::ToggledButtonWithTooltip("hb##showhb", &showHurtbox, "Show Hurtbox");
    ImGui::SameLine();
    ImGui::ToggledButtonWithTooltip("FO##showFO", &showFrameObjects, "Show Frame Objects");
    ImGui::SameLine();
    ImGui::ToggledButtonWithTooltip("AP##showAP", &showActionPoints, "Show Action Points");
    ImGui::SameLine();
}

static void exportOrderTree(const std::vector<AbstractMetaSpriteEditorGui::ExportOrderTree>& tree,
                            AbstractMetaSpriteEditorGui* gui,
                            std::function<void(AbstractMetaSpriteEditorGui*, const idstring&)> addFunction,
                            const std::string& addSuffix)
{
    static unsigned contextMenuIndex = INT_MAX;

    for (auto [i, eo] : enumerate(tree)) {
        ImGui::PushStyleColor(ImGuiCol_Text, Style::successFailColor(eo.valid));
        ImGui::TextUnformatted(eo.valid ? u8"·" : u8"X");
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::TextUnformatted(eo.name);

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

            for (auto [i, alt] : const_enumerate(eo.alternatives)) {
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

    ImGui::SetNextWindowSize(ImVec2(325, 650), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(windowLabel)) {

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
