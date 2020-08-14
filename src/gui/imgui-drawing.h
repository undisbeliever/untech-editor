/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "vendor/imgui/imgui.h"
#include <cmath>

namespace UnTech::Gui {

static inline ImVec2 centreOffset(const ImVec2& size)
{
    const ImVec2 region = ImGui::GetContentRegionAvail();

    ImVec2 offset = ImGui::GetCursorScreenPos();
    if (region.x > size.x) {
        offset.x += std::floor((region.x - size.x) / 2);
    }
    if (region.y > size.y) {
        offset.y += std::floor((region.y - size.y) / 2);
    }
    return offset;
}

static inline ImVec2 operator+(const ImVec2& a, const ImVec2& b)
{
    return ImVec2(a.x + b.x, a.y + b.y);
}
static inline ImVec2 operator-(const ImVec2& a, const ImVec2& b)
{
    return ImVec2(a.x - b.x, a.y - b.y);
}

}
