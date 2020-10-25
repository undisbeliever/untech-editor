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

enum EditListAction {
    ADD,
    CLONE,
    REMOVE,
    RAISE_TO_TOP,
    RAISE,
    LOWER,
    LOWER_TO_BOTTOM,
};

template <typename ActionPolicy>
struct ListActions;

// ::TODO add symbols to buttons::

template <typename ActionPolicy>
bool ListButtons(typename ActionPolicy::EditorT* editor)
{
    assert(editor != nullptr);

    bool listChanged = false;

    if (ImGui::ButtonWithTooltip("A", "Add")) {
        ListActions<ActionPolicy>::editList(editor, EditListAction::ADD);
        listChanged = true;
    }
    ImGui::SameLine();

    if (ImGui::ButtonWithTooltip("C", "Clone Selected")) {
        ListActions<ActionPolicy>::editList(editor, EditListAction::CLONE);
        listChanged = true;
    }
    ImGui::SameLine();

    if (ImGui::ButtonWithTooltip("R##Remove", "Remove Selected")) {
        ListActions<ActionPolicy>::editList(editor, EditListAction::REMOVE);
        listChanged = true;
    }
    ImGui::SameLine();

    if (ImGui::ButtonWithTooltip("Rt", "Raise to Top")) {
        ListActions<ActionPolicy>::editList(editor, EditListAction::RAISE_TO_TOP);
        listChanged = true;
    }
    ImGui::SameLine();

    if (ImGui::ButtonWithTooltip("R##Raise", "Raise")) {
        ListActions<ActionPolicy>::editList(editor, EditListAction::RAISE);
        listChanged = true;
    }
    ImGui::SameLine();

    if (ImGui::ButtonWithTooltip("L", "Lower")) {
        ListActions<ActionPolicy>::editList(editor, EditListAction::LOWER);
        listChanged = true;
    }
    ImGui::SameLine();

    if (ImGui::ButtonWithTooltip("Lb", "Lower to Bottom")) {
        ListActions<ActionPolicy>::editList(editor, EditListAction::LOWER_TO_BOTTOM);
        listChanged = true;
    }

    return listChanged;
}

template <typename ActionPolicy>
bool CombinedListButtons_AddButton(typename ActionPolicy::EditorT* editor)
{
    bool changed = false;

    ImGui::PushID((void*)ActionPolicy::name);

    if (ImGui::Button("A")) {
        ListActions<ActionPolicy>::editList(editor, EditListAction::ADD);
        changed = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("Add %s", ActionPolicy::name);
        ImGui::EndTooltip();
    }
    ImGui::PopID();

    ImGui::SameLine();

    return changed;
}

template <typename... ActionPolicy, typename EditorT>
bool CombinedListButtons(const char* idStr, EditorT* editor)
{
    assert(editor != nullptr);

    ImGui::PushID(idStr);

    bool listChanged = false;

    listChanged = (CombinedListButtons_AddButton<ActionPolicy>(editor) | ...);

    if (ImGui::ButtonWithTooltip("C", "Clone Selected")) {
        editor->startMacro();
        ((ListActions<ActionPolicy>::editList(editor, EditListAction::CLONE)), ...);
        editor->endMacro();
        listChanged = true;
    }
    ImGui::SameLine();

    if (ImGui::ButtonWithTooltip("R##Remove", "Remove Selected")) {
        editor->startMacro();
        ((ListActions<ActionPolicy>::editList(editor, EditListAction::REMOVE)), ...);
        editor->endMacro();
        listChanged = true;
    }
    ImGui::SameLine();

    if (ImGui::ButtonWithTooltip("RT", "Raise to Top")) {
        editor->startMacro();
        ((ListActions<ActionPolicy>::editList(editor, EditListAction::RAISE_TO_TOP)), ...);
        editor->endMacro();
        listChanged = true;
    }
    ImGui::SameLine();

    if (ImGui::ButtonWithTooltip("R##Raise", "Raise")) {
        editor->startMacro();
        ((ListActions<ActionPolicy>::editList(editor, EditListAction::RAISE)), ...);
        editor->endMacro();
        listChanged = true;
    }
    ImGui::SameLine();

    if (ImGui::ButtonWithTooltip("L", "Lower")) {
        editor->startMacro();
        ((ListActions<ActionPolicy>::editList(editor, EditListAction::LOWER)), ...);
        editor->endMacro();
        listChanged = true;
    }
    ImGui::SameLine();

    if (ImGui::ButtonWithTooltip("LB", "Lower to Bottom")) {
        editor->startMacro();
        ((ListActions<ActionPolicy>::editList(editor, EditListAction::LOWER_TO_BOTTOM)), ...);
        editor->endMacro();
        listChanged = true;
    }

    ImGui::PopID();

    return listChanged;
}

template <class ActionPolicy>
void NamedListSidebar(typename ActionPolicy::EditorT* editor, float width = 200)
{
    using Actions = ListActions<ActionPolicy>;
    using SelectionT = typename ActionPolicy::SelectionT;

    ImGui::BeginChild("name-list", ImVec2(width, 0), true);
    {
        ListButtons<ActionPolicy>(editor);

        const NamedList<typename ActionPolicy::ListT::value_type>* list = Actions::getEditorListPtr(editor, std::make_tuple());

        ImGui::BeginChild("struct-list");

        if (list) {
            SelectionT& sel = editor->*ActionPolicy::SelectionPtr;

            for (unsigned i = 0; i < list->size(); i++) {
                const auto& item = list->at(i);

                ImGui::PushID(i);

                ImGui::Selectable("##sel", &sel, i, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick);
                ImGui::SameLine();
                ImGui::TextUnformatted(item.name);

                ImGui::PopID();
            }
        }
        ImGui::EndChild();
    }
    ImGui::EndChild();
}

}
