/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "imgui.h"
#include "selection.h"
#include "models/common/namedlist.h"

namespace UnTech::Gui {

// ::TODO connect with undo system::
template <typename T>
void ListButtons(SingleSelection* sel, T* list, const unsigned maxSize)
{
    assert(maxSize <= SingleSelection::MAX_SIZE);

    if (ImGui::Button("Add")) {
        assert(list != nullptr);

        if (list->size() < maxSize) {
            list->emplace_back();

            sel->selected = list->size() - 1;
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Remove")) {
        assert(list != nullptr);

        if (sel->selected < list->size()) {
            list->erase(list->begin() + sel->selected);
            sel->selected = SingleSelection::NO_SELECTION;
        }
    }
}

// ::TODO connect with undo system::
template <typename T>
void ListButtons(MultipleSelection* sel, T* list, const unsigned maxSize = 64)
{
    assert(maxSize <= MultipleSelection::MAX_SIZE);

    if (ImGui::Button("Add")) {
        assert(list != nullptr);

        if (list->size() < maxSize) {
            list->emplace_back();

            sel->selected = 1 << (list->size() - 1);
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Remove")) {
        assert(list != nullptr);
        const auto s = sel->selected;

        if (s != 0) {
            // ::TODO use std::bit_width when upgrading c++20::
            const size_t last = std::min<size_t>(list->size(), maxSize);

            for (size_t i = last; i > 0; i--) {
                unsigned toRemove = i - 1;

                if (s & (1 << toRemove)) {
                    list->erase(list->begin() + toRemove);
                }
            }
        }
        sel->selected = SingleSelection::NO_SELECTION;
    }
}

template <class T>
void NamedListSidebar(NamedList<T>& list, SingleSelection* sel, unsigned maxSize, float width = 200)
{
    ImGui::BeginChild("name-list", ImVec2(width, 0), true);
    {
        ListButtons(sel, &list, maxSize);

        ImGui::BeginChild("struct-list");

        for (unsigned i = 0; i < list.size(); i++) {
            const auto& item = list.at(i);

            ImGui::PushID(i);

            ImGui::Selectable("##sel", sel, i, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick);
            ImGui::SameLine();
            ImGui::TextUnformatted(item.name);

            ImGui::PopID();
        }
        ImGui::EndChild();

        UpdateSelection(sel);
    }
    ImGui::EndChild();
}

}
