/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "splitter.h"
#include "imgui-drawing.h"
#include "vendor/imgui/imgui_internal.h"
#include <algorithm>

namespace UnTech::Gui {

static bool VerticalSplitter(const char* strId, float* width1, float* width2, float minWidth1, float minWidth2, float splitterHeight)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    const ImGuiID id = window->GetID(strId);

    ImRect bb;
    bb.Min = window->DC.CursorPos + ImVec2(*width1, 0.0f);
    bb.Max = bb.Min + ImVec2(SPLITTER_THICKNESS, splitterHeight);

    return ImGui::SplitterBehavior(bb, id, ImGuiAxis_X, width1, width2, minWidth1, minWidth2);
}

static bool HorizontalSplitter(const char* strId, float* height1, float* height2, float minHeight1, float minHeight2, float splitterWidth)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    const ImGuiID id = window->GetID(strId);

    ImRect bb;
    bb.Min = window->DC.CursorPos + ImVec2(0.0f, *height1);
    bb.Max = bb.Min + ImVec2(splitterWidth, SPLITTER_THICKNESS);

    return ImGui::SplitterBehavior(bb, id, ImGuiAxis_Y, height1, height2, minHeight1, minHeight2);
}

template <bool ON_LEFT>
static std::pair<ImVec2, ImVec2> splitterBarLeftRight(const char* strId, SplitterBarState* state)
{
    const auto region = ImGui::GetContentRegionAvail();
    const float height = region.y;

    // The *2 removes an unwanted scrollbar that can appear in `splitterTopbar()` and `splitterBottombar()`
    // Also improves the right padding of `splitterSidebarRight()`.
    float otherWidth = region.x - SPLITTER_THICKNESS * 2 - state->barSize;
    if (otherWidth < state->minOtherSize) {
        otherWidth = state->minOtherSize;
        state->barSize = region.x - SPLITTER_THICKNESS - otherWidth;
    }
    if (state->barSize < state->minBarSize) {
        state->barSize = state->minBarSize;
    }

    if constexpr (ON_LEFT) {
        VerticalSplitter(strId, &state->barSize, &otherWidth, state->minBarSize, state->minOtherSize, height);
        return { ImVec2(state->barSize, height), ImVec2(otherWidth, height) };
    }
    else {
        VerticalSplitter(strId, &otherWidth, &state->barSize, state->minOtherSize, state->minBarSize, height);
        return { ImVec2(otherWidth, height), ImVec2(state->barSize, height) };
    }
}

template <bool ON_TOP>
static std::pair<ImVec2, ImVec2> splitterBarTopBottom(const char* strId, SplitterBarState* state)
{
    const auto region = ImGui::GetContentRegionAvail();
    const float width = region.x;

    // *2 to match `splitterBarLeftRight()`
    float otherHeight = region.y - SPLITTER_THICKNESS * 2 - state->barSize;
    if (otherHeight < state->minOtherSize) {
        otherHeight = state->minOtherSize;
        state->barSize = region.x - SPLITTER_THICKNESS - otherHeight;
    }
    if (state->barSize < state->minBarSize) {
        state->barSize = state->minBarSize;
    }

    if constexpr (ON_TOP) {
        HorizontalSplitter(strId, &state->barSize, &otherHeight, state->minBarSize, state->minOtherSize, width);
        return { ImVec2(width, state->barSize), ImVec2(width, otherHeight) };
    }
    else {
        HorizontalSplitter(strId, &otherHeight, &state->barSize, state->minOtherSize, state->minBarSize, width);
        return { ImVec2(width, otherHeight), ImVec2(width, state->barSize) };
    }
}

std::pair<ImVec2, ImVec2> splitterSidebarLeft(const char* strId, SplitterBarState* state)
{
    return splitterBarLeftRight<true>(strId, state);
}

std::pair<ImVec2, ImVec2> splitterSidebarRight(const char* strId, SplitterBarState* state)
{
    return splitterBarLeftRight<false>(strId, state);
}

std::pair<ImVec2, ImVec2> splitterTopbar(const char* strId, SplitterBarState* state)
{
    return splitterBarTopBottom<true>(strId, state);
}

std::pair<ImVec2, ImVec2> splitterBottombar(const char* strId, SplitterBarState* state)
{
    return splitterBarTopBottom<false>(strId, state);
}

}
