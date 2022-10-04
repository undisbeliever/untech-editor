/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstract-metasprite-editor.h"
#include "gui/style.h"
#include "models/common/iterators.h"
#include "models/common/stringbuilder.h"
#include <functional>

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

void AbstractMetaSpriteEditorGui::setMetaSpriteData(AbstractMetaSpriteEditorData* data)
{
    _animationState.resetState();
    _animationTimer.stop();

    if (data) {
        _animationState.animationIndex = data->animationsSel.selectedIndex();
    }
}

void AbstractMetaSpriteEditorGui::viewMenu()
{
    ImGui::MenuItem("Show Tile Hitbox", nullptr, &showTileHitbox);
    ImGui::MenuItem("Show Shield Box", nullptr, &showShield);
    ImGui::MenuItem("Show Hitbox", nullptr, &showHitbox);
    ImGui::MenuItem("Show Hurtbox", nullptr, &showHurtbox);
    ImGui::MenuItem("Show Frame Objects", nullptr, &showFrameObjects);
    ImGui::MenuItem("Show Action Points", nullptr, &showActionPoints);

    ImGui::Separator();

    ImGui::MenuItem("Animation Preview Window", nullptr, &showAnimationPreviewWindow);
    ImGui::MenuItem("Export Order Window", nullptr, &showExportOrderWindow);
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

void AbstractMetaSpriteEditorGui::showExtraWindowButtons()
{
    ImGui::ToggledButton("Animation Preview", &showAnimationPreviewWindow);
    ImGui::SameLine();
    ImGui::ToggledButton("Export Order", &showExportOrderWindow);
}

static void exportOrderTree(const std::vector<AbstractMetaSpriteEditorGui::ExportOrderTree>& tree,
                            AbstractMetaSpriteEditorGui* gui,
                            std::function<void(AbstractMetaSpriteEditorGui*, const idstring&)> addFunction,
                            const std::u8string& addSuffix)
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
                const std::u8string text = stringBuilder(u8"Add ", name, addSuffix);
                if (ImGui::MenuItem(u8Cast(text))) {
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

void AbstractMetaSpriteEditorGui::exportOrderWindow()
{
    using namespace std::string_literals;

    if (!showExportOrderWindow) {
        return;
    }

    ImGui::SetNextWindowPos(ImVec2(310, 85), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(250, 620), ImGuiCond_Once);

    if (ImGui::Begin("Export Order##AMS", &showExportOrderWindow)) {

        if (!_eoStillFrames.empty() || !_eoAnimations.empty()) {
            if (ImGui::TreeNodeEx("Still Frames", ImGuiTreeNodeFlags_DefaultOpen)) {
                exportOrderTree(_eoStillFrames, this, &AbstractMetaSpriteEditorGui::addFrame, u8" Frame"s);
                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("Animations", ImGuiTreeNodeFlags_DefaultOpen)) {
                exportOrderTree(_eoAnimations, this, &AbstractMetaSpriteEditorGui::addAnimation, u8" Animation"s);
                ImGui::TreePop();
            }
        }
    }
    ImGui::End();
}

void durationFormatText(const MetaSprite::Animation::DurationFormat df, uint8_t duration)
{
    using DurationFormat = UnTech::MetaSprite::Animation::DurationFormat;

    if (duration == 0) {
        duration = 1;
    }

    switch (df) {
    case DurationFormat::FRAME:
        ImGui::Text("%d frames", int(duration));
        break;

    case DurationFormat::TIME:
        ImGui::Text("%d ms", int(duration * 1000 / 75));
        break;

    case DurationFormat::DISTANCE_HORIZONTAL:
    case DurationFormat::DISTANCE_VERTICAL:
        ImGui::Text("%0.3f px", double(duration) / 32);
        break;
    }
}

}
