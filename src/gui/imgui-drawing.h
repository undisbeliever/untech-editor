/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "vendor/imgui/imgui.h"
#include <cmath>

namespace UnTech::Gui {

// This function preforms 3 different actions:
//      1) Expand the region if it is smaller than `size` (using an InvisibleButton).
//      2) Cover the entire region with an InvisibleButton to capture mouse inputs.
//      3) Calculate the screen offset to draw the canvas to (centering the offset if `size` > content-region).
static inline ImVec2 captureMouseExpandCanvasAndCalcScreenPos(const char* strId, const ImVec2& size)
{
    const ImVec2 region = ImGui::GetContentRegionAvail();

    ImVec2 offset = ImGui::GetCursorScreenPos();
    if (region.x > size.x) {
        offset.x += std::floor((region.x - size.x) / 2);
    }
    if (region.y > size.y) {
        offset.y += std::floor((region.y - size.y) / 2);
    }

    ImGui::InvisibleButton(strId, ImVec2(std::max(region.x, size.x), std::max(region.y, size.y)));

    return offset;
}

static inline ImVec2 operator+(const ImVec2& a, const ImVec2& b)
{
    return { a.x + b.x, a.y + b.y };
}
static inline ImVec2 operator-(const ImVec2& a, const ImVec2& b)
{
    return { a.x - b.x, a.y - b.y };
}

}
