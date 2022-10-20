/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "imgui.h"
#include "list-actions.h"
#include "selection.h"
#include "models/common/namedlist.h"

namespace UnTech::Gui {

template <typename ActionPolicy>
void ListButtons(typename ActionPolicy::EditorT* editor)
{
    assert(editor != nullptr);

    if (ImGui::ButtonWithTooltip("A", "Add")) {
        ListActions<ActionPolicy>::editList(editor, EditListAction::ADD);
    }
    ImGui::SameLine();

    if (ImGui::ButtonWithTooltip("C", "Clone Selected")) {
        ListActions<ActionPolicy>::editList(editor, EditListAction::CLONE);
    }
    ImGui::SameLine();

    if (ImGui::ButtonWithTooltip("R##Remove", "Remove Selected")) {
        ListActions<ActionPolicy>::editList(editor, EditListAction::REMOVE);
    }
    ImGui::SameLine();

    if (ImGui::ButtonWithTooltip("Rt", "Raise to Top")) {
        ListActions<ActionPolicy>::editList(editor, EditListAction::RAISE_TO_TOP);
    }
    ImGui::SameLine();

    if (ImGui::ButtonWithTooltip("R##Raise", "Raise")) {
        ListActions<ActionPolicy>::editList(editor, EditListAction::RAISE);
    }
    ImGui::SameLine();

    if (ImGui::ButtonWithTooltip("L", "Lower")) {
        ListActions<ActionPolicy>::editList(editor, EditListAction::LOWER);
    }
    ImGui::SameLine();

    if (ImGui::ButtonWithTooltip("Lb", "Lower to Bottom")) {
        ListActions<ActionPolicy>::editList(editor, EditListAction::LOWER_TO_BOTTOM);
    }
}

template <typename ActionPolicy>
bool CombinedListButtons_AddButton(typename ActionPolicy::EditorT* editor)
{
    bool changed = false;

    ImGui::PushID(ActionPolicy::name);

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
void CombinedListButtons(const char* idStr, EditorT* editor)
{
    assert(editor != nullptr);

    ImGui::PushID(idStr);

    (CombinedListButtons_AddButton<ActionPolicy>(editor), ...);

    if (ImGui::ButtonWithTooltip("C", "Clone Selected")) {
        editor->undoStack().startMacro();
        ((ListActions<ActionPolicy>::editList(editor, EditListAction::CLONE)), ...);
        editor->undoStack().endMacro();
    }
    ImGui::SameLine();

    if (ImGui::ButtonWithTooltip("R##Remove", "Remove Selected")) {
        editor->undoStack().startMacro();
        ((ListActions<ActionPolicy>::editList(editor, EditListAction::REMOVE)), ...);
        editor->undoStack().endMacro();
    }
    ImGui::SameLine();

    if (ImGui::ButtonWithTooltip("RT", "Raise to Top")) {
        editor->undoStack().startMacro();
        ((ListActions<ActionPolicy>::editList(editor, EditListAction::RAISE_TO_TOP)), ...);
        editor->undoStack().endMacro();
    }
    ImGui::SameLine();

    if (ImGui::ButtonWithTooltip("R##Raise", "Raise")) {
        editor->undoStack().startMacro();
        ((ListActions<ActionPolicy>::editList(editor, EditListAction::RAISE)), ...);
        editor->undoStack().endMacro();
    }
    ImGui::SameLine();

    if (ImGui::ButtonWithTooltip("L", "Lower")) {
        editor->undoStack().startMacro();
        ((ListActions<ActionPolicy>::editList(editor, EditListAction::LOWER)), ...);
        editor->undoStack().endMacro();
    }
    ImGui::SameLine();

    if (ImGui::ButtonWithTooltip("LB", "Lower to Bottom")) {
        editor->undoStack().startMacro();
        ((ListActions<ActionPolicy>::editList(editor, EditListAction::LOWER_TO_BOTTOM)), ...);
        editor->undoStack().endMacro();
    }

    ImGui::PopID();
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

            for (auto [i, item] : enumerate(*list)) {
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
