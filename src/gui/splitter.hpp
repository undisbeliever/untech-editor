/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "splitter.h"

namespace UnTech::Gui {

template <typename LeftFunction, typename RightFunction>
inline void splitterSidebarLeft(const char* strId_splitter, SplitterBarState* state,
                                const char* strId_left, LeftFunction f1,
                                const char* strId_right, RightFunction f2)
{
    const auto s = splitterSidebarLeft(strId_splitter, state);

    ImGui::BeginChild(strId_left, s.first, false);
    f1();
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild(strId_right, s.second, false);
    f2();
    ImGui::EndChild();
}

template <typename LeftFunction, typename RightFunction>
inline void splitterSidebarRight(const char* strId_splitter, SplitterBarState* state,
                                 const char* strId_left, LeftFunction f1,
                                 const char* strId_right, RightFunction f2)
{
    const auto s = splitterSidebarRight(strId_splitter, state);

    ImGui::BeginChild(strId_left, s.first, false);
    f1();
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild(strId_right, s.second, false);
    f2();
    ImGui::EndChild();
}

template <typename TopFunction, typename BottomFunction>
inline void splitterTopbar(const char* strId_splitter, SplitterBarState* state,
                           const char* strId_top, TopFunction f1,
                           const char* strId_bottom, BottomFunction f2)
{
    const auto s = splitterTopbar(strId_splitter, state);

    ImGui::BeginChild(strId_top, s.first, false);
    f1();
    ImGui::EndChild();

    ImGui::BeginChild(strId_bottom, s.second, false);
    f2();
    ImGui::EndChild();
}

template <typename Top1Function, typename Top2Function, typename BottomFunction>
inline void splitterTopbar3(const char* strId_splitter1, const char* strId_splitter2,
                            SplitterBarState* state1, SplitterBarState* state2,
                            const char* strId_top1, Top1Function f1,
                            const char* strId_top2, Top2Function f2,
                            const char* strId_bottom, BottomFunction f3)
{
    const auto s1 = splitterTopbar(strId_splitter1, state1);

    ImGui::BeginChild(strId_top1, s1.first, false);
    f1();
    ImGui::EndChild();

    const auto s2 = splitterTopbar(strId_splitter2, state2);

    ImGui::BeginChild(strId_top2, s2.first, false);
    f2();
    ImGui::EndChild();

    ImGui::BeginChild(strId_bottom, s2.second, false);
    f3();
    ImGui::EndChild();
}

template <typename TopFunction, typename BottomFunction>
inline void splitterBottombar(const char* strId_splitter, SplitterBarState* state,
                              const char* strId_top, TopFunction f1,
                              const char* strId_bottom, BottomFunction f2)
{
    const auto s = splitterBottombar(strId_splitter, state);

    ImGui::BeginChild(strId_top, s.first, false);
    f1();
    ImGui::EndChild();

    ImGui::BeginChild(strId_bottom, s.second, false);
    f2();
    ImGui::EndChild();
}

}
