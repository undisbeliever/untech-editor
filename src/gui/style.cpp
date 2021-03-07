/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "style.h"
#include "imgui-drawing.h"
#include "models/common/stringbuilder.h"
#include "vendor/imgui/imgui_internal.h"

namespace UnTech::Gui {

const ImVec4 Style::tileCollisionTint(0.75f, 0, 0.75f, 0.5);
const ImVec4 Style::tilePropertiesButtonTint(0.5f, 0.5f, 0.5f, 1.0f);
const ImVec4 Style::tilePropertiesButtonSelectedTint(1.0f, 1.0f, 1.0f, 1.0f);

const ImVec4 Style::commentColorDark = { 0.25f, 1.0f, 0.25f, 1.0f };
const ImVec4 Style::commentColorLight = { 0.0f, 0.45f, 0.0f, 1.0f };

Zoom Style::metaSpriteAnimationZoom(600);
Zoom Style::metaSpriteZoom(600);
Zoom Style::spriteImporterZoom(600);
Zoom Style::backgroundImageZoom(200);
Zoom Style::metaTileTilesetZoom(300);
Zoom Style::metaTileScratchpadZoom(300);
Zoom Style::roomEditorZoom(300);

ZoomAspectRatio Style::aspectRatio = ZoomAspectRatio::Ntsc;

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
    z = std::clamp<unsigned>(z, 5, 1800);

    _zoomInt = z;

    update();
}

static float aspectRatioScale(const ZoomAspectRatio ar)
{
    // Values taken from bsnes-plus
    switch (ar) {
    case ZoomAspectRatio::Ntsc:
        return 54.0f / 47.0f;

    case ZoomAspectRatio::Pal:
        return 32.0f / 23.0f;

    case ZoomAspectRatio::Square:
        return 1.0f;
    }

    return 1.0f;
}

void Zoom::update()
{
    const float scale = aspectRatioScale(Style::aspectRatio);

    _zoom.x = _zoomInt / 100.0f * scale;
    _zoom.y = _zoomInt / 100.0f;

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

const ImVec4& Style::commentColor()
{
    const bool isDark = ImGui::GetStyleColorVec4(ImGuiCol_Text).x > 0.5f;
    return isDark ? commentColorDark : commentColorLight;
}

void Style::setAspectRatio(ZoomAspectRatio ar)
{
    aspectRatio = ar;

    metaSpriteAnimationZoom.update();
    metaSpriteZoom.update();
    spriteImporterZoom.update();
    backgroundImageZoom.update();
    metaTileTilesetZoom.update();
    metaTileScratchpadZoom.update();
    roomEditorZoom.update();
}

}
