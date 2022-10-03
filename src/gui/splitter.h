/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "vendor/imgui/imgui.h"
#include <tuple>

namespace UnTech::Gui {

constexpr float SPLITTER_THICKNESS = 3.0f;

struct SplitterBarState {
    float barSize;
    const float minBarSize;
    const float minOtherSize;
};

std::pair<ImVec2, ImVec2> splitterSidebarLeft(const char* strId, SplitterBarState* state);
std::pair<ImVec2, ImVec2> splitterSidebarRight(const char* strId, SplitterBarState* state);

std::pair<ImVec2, ImVec2> splitterTopbar(const char* strId, SplitterBarState* state);
std::pair<ImVec2, ImVec2> splitterBottombar(const char* strId, SplitterBarState* state);

}
