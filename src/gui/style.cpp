/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "style.h"
#include "imgui-drawing.h"
#include "models/common/stringbuilder.h"
#include "vendor/imgui/imgui_internal.h"

namespace UnTech::Gui {

const ImVec4 Style::tilePropertiesButtonTint(0.5f, 0.5f, 0.5f, 1.0f);
const ImVec4 Style::tilePropertiesButtonSelectedTint(1.0f, 1.0f, 1.0f, 1.0f);

Zoom Style::metaSpriteZoom(600);
Zoom Style::spriteImporterZoom(600);
Zoom Style::backgroundImageZoom(200);
Zoom Style::metaTileTilesetZoom(300);
Zoom Style::metaTileScratchpadZoom(300);
Zoom Style::roomEditorZoom(300);

const std::array<std::pair<unsigned, const char*>, 19> zoomComboItems{ {
    { 25, "25%" },
    { 50, "50%" },
    { 75, "75%" },
    { 100, "100%" },
    { 150, "150%" },
    { 200, "200%" },
    { 300, "300%" },
    { 400, "400%" },
    { 500, "500%" },
    { 600, "600%" },
    { 700, "700%" },
    { 800, "800%" },
    { 900, "900%" },
    { 1000, "1000%" },
    { 1200, "1200%" },
    { 1400, "1500%" },
    { 1600, "1600%" },
    { 1800, "1800%" },
    { 2000, "2000%" },
} };

Zoom::Zoom(unsigned z)
{
    setZoom(z);
}

void Zoom::setZoom(unsigned z)
{
    // ::TODO add aspect ratio::

    z = std::clamp<unsigned>(z, 5, 1800);

    _zoomInt = z;
    _zoom.x = z / 100.0f;
    _zoom.y = z / 100.0f;

    _zoomString = stringBuilder(_zoomInt, "%");
}

void Zoom::zoomCombo(const char* label)
{
    ImGui::SetNextItemWidth(100);
    if (ImGui::BeginCombo(label, _zoomString.c_str())) {
        for (auto& [z, str] : zoomComboItems) {
            if (ImGui::Selectable(str, _zoomInt == z)) {
                setZoom(z);
            }
        }
        ImGui::EndCombo();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted("Zoom");
        ImGui::EndTooltip();
    }
}

void Zoom::processMouseWheel()
{
    const auto& io = ImGui::GetIO();

    auto updateZoom = [&](unsigned z) {
        if (_zoomInt == z) {
            return;
        }

        const ImVec2 oldZoom = _zoom;
        setZoom(z);

        const ImVec2 windowSize = ImGui::GetWindowSize();
        const ImVec2 halfWindowSize(windowSize.x / 2, windowSize.y / 2);

        ImGui::SetScrollX((ImGui::GetScrollX() + halfWindowSize.x) * _zoom.x / oldZoom.x - halfWindowSize.x);
        ImGui::SetScrollY((ImGui::GetScrollY() + halfWindowSize.y) * _zoom.y / oldZoom.y - halfWindowSize.y);
    };

    if (ImGui::IsWindowHovered()) {
        if (io.KeyCtrl) {
            if (io.MouseWheel < 0.0f) {
                unsigned z = zoomComboItems.front().first;
                auto it = std::find_if(zoomComboItems.begin(), zoomComboItems.end(),
                                       [&](auto& z) { return z.first >= _zoomInt; });
                if (it != zoomComboItems.begin()) {
                    it--;
                    z = it->first;
                }
                updateZoom(z);
            }
            else if (io.MouseWheel > 0.0f) {
                unsigned z = zoomComboItems.back().first;
                for (unsigned i = 0; i < zoomComboItems.size() - 1; i++) {
                    if (zoomComboItems.at(i).first >= zoomInt()) {
                        z = zoomComboItems.at(i + 1).first;
                        break;
                    }
                }
                updateZoom(z);
            }
        }
    }
}

}
