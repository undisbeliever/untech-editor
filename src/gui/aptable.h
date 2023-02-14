/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2022, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/imgui-combos.h"
#include "gui/imgui.h"
#include "gui/list-actions.h"
#include "gui/list-helpers.h"
#include "models/metasprite/common.h"
#include <type_traits>

namespace UnTech::Gui {

// The Cell functions return true when the item has finished editing

inline bool Cell(const char* label, idstring* input)
{
    ImGui::InputIdstring(label, input);
    return ImGui::IsItemDeactivatedAfterEdit();
}

inline bool Cell(const char* label, std::u8string* input)
{
    ImGui::InputText(label, input);
    return ImGui::IsItemDeactivatedAfterEdit();
}

inline bool Cell(const char* label, uint8_t* v)
{
    ImGui::InputUint8(label, v, 0);
    return ImGui::IsItemDeactivatedAfterEdit();
}

inline bool Cell(const char* label, uint16_t* v)
{
    ImGui::InputUint16(label, v, 0);
    return ImGui::IsItemDeactivatedAfterEdit();
}

inline bool Cell_Formatted(const char* label, uint32_t* v, const char* format, ImGuiInputTextFlags flags = 0)
{
    ImGui::InputUnsignedFormat(label, v, format, flags);
    return ImGui::IsItemDeactivatedAfterEdit();
}

inline bool Cell(const char* label, unsigned* v)
{
    ImGui::InputUnsigned(label, v, 0);
    return ImGui::IsItemDeactivatedAfterEdit();
}

inline bool Cell(const char* label, unsigned* v, unsigned maxValue)
{
    bool modified = ImGui::InputUnsigned(label, v, 0);
    if (modified) {
        *v = std::min(*v, maxValue);
    }
    return ImGui::IsItemDeactivatedAfterEdit();
}

inline bool Cell(const char* label, MetaSprite::SpriteOrderType* v)
{
    ImU32 i = *v;
    const bool modified = ImGui::InputUnsigned(label, &i, 0);
    if (modified) {
        *v = i;
    }
    return ImGui::IsItemDeactivatedAfterEdit();
}

inline bool Cell(const char* label, point* v, const rect& bounds)
{
    ImGui::InputPoint(label, v, bounds);
    return ImGui::IsItemDeactivatedAfterEdit();
}

inline bool Cell(const char* label, upoint* v, const usize& bounds)
{
    ImGui::InputUpoint(label, v, bounds);
    return ImGui::IsItemDeactivatedAfterEdit();
}

inline bool Cell(const char* label, upoint* v, const usize& containerSize, const usize& itemSize)
{
    ImGui::InputUpoint(label, v, containerSize, itemSize);
    return ImGui::IsItemDeactivatedAfterEdit();
}

inline bool Cell(const char* label, usize* v, const usize& maxSize)
{
    ImGui::InputUsize(label, v, maxSize);
    return ImGui::IsItemDeactivatedAfterEdit();
}

inline bool Cell(const char* label, urect* v, const usize& bounds)
{
    ImGui::InputUrect(label, v, bounds);
    return ImGui::IsItemDeactivatedAfterEdit();
}

inline bool Cell(const char* label, ms8point* v)
{
    ImGui::InputMs8point(label, v);
    return ImGui::IsItemDeactivatedAfterEdit();
}

inline bool Cell(const char* label, ms8rect* v)
{
    ImGui::InputMs8rect(label, v);
    return ImGui::IsItemDeactivatedAfterEdit();
}

inline bool Cell(const char* label, rgba* v)
{
    ImGui::InputRgb(label, v);
    return ImGui::IsItemDeactivatedAfterEdit();
}

// ----------------

inline bool Cell(const char* label, idstring* input, const std::vector<idstring>& list, bool includeBlank = false)
{
    return ImGui::IdStringCombo(label, input, list, includeBlank);
}

template <typename T>
inline bool Cell(const char* label, idstring* input, const NamedList<T>& list, bool includeBlank = false)
{
    return ImGui::IdStringCombo(label, input, list, includeBlank);
}

template <typename T>
inline bool Cell(const char* label, idstring* input, const ExternalFileList<T>& list, bool includeBlank = false)
{
    return ImGui::IdStringCombo(label, input, list, includeBlank);
}

template <typename T>
    requires std::is_enum_v<T>
inline bool Cell(const char* label, T* v)
{
    return ImGui::EnumCombo(label, v);
}

inline bool Cell(const char* label, Scripting::ComparisonType* v, Scripting::ConditionalType t)
{
    return ImGui::EnumCombo(label, v, t);
}

inline bool Cell(const char* label, bool* v)
{
    return ImGui::Checkbox(label, v);
}

inline bool Cell_FlipCombo(const char* label, bool* hFlip, bool* vFlip)
{
    return ImGui::FlipCombo(label, hFlip, vFlip);
}

// ----------------

template <size_t N_COLUMNS>
inline bool beginApTable(const char* strId, const std::array<const char*, N_COLUMNS>& columnNames, const ImVec2& outerSize = ImVec2(0.0f, 0.0f))
{
    constexpr auto tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_ScrollY;

    const bool b = ImGui::BeginTable(strId, N_COLUMNS + 1, tableFlags, outerSize);

    if (b) {
        ImGui::TableSetupScrollFreeze(0, 1);

        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 30.0f);
        for (const char* c : columnNames) {
            ImGui::TableSetupColumn(c);
        }

        ImGui::TableHeadersRow();
    }

    return b;
}

template <size_t N_COLUMNS>
inline bool beginApTable_noScrolling(const char* strId, const std::array<const char*, N_COLUMNS>& columnNames)
{
    constexpr auto tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings;

    const bool b = ImGui::BeginTable(strId, N_COLUMNS + 1, tableFlags);

    if (b) {
        ImGui::TableSetupScrollFreeze(0, 1);

        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 30.0f);
        for (const char* c : columnNames) {
            ImGui::TableSetupColumn(c);
        }

        ImGui::TableHeadersRow();
    }

    return b;
}

inline void endApTable() { ImGui::EndTable(); }

// ----------------

// The `columnFunctions` take a `value_type&` input and return `true` when the item has finished editing
template <typename ActionPolicy, typename EditorT, typename... T>
inline void apTable_data_custom(const std::shared_ptr<EditorT>& editor, const std::tuple<T...> listArgs,
                                auto selFunction, auto... columnFunctions)
{
    // ::TODO improve selection::

    auto* data = ActionPolicy::getEditorData(*editor);
    assert(data);

    auto& sel = (*editor).*(ActionPolicy::SelectionPtr);
    auto* list = std::apply(&ActionPolicy::getList,
                            std::tuple_cat(std::forward_as_tuple(*data), listArgs));

    if (list) {
        for (auto p : enumerate(*list)) {
            const auto& index = p.first;
            typename ActionPolicy::ListT::value_type& item = p.second;

            ImGui::TableNextRow();

            ImGui::PushID(index);

            ImGui::TableNextColumn();
            selFunction(&sel, index);

            bool edited = false;

            auto processColumn = [&](const auto& cf) {
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1);
                const auto itemEdited = cf(item);

                static_assert(std::is_same_v<decltype(itemEdited), const bool>, "columnFunction must return a bool");
                edited |= itemEdited;
            };
            (processColumn(columnFunctions), ...);

            if (edited) {
                AbstractListActions<ActionPolicy>::itemEdited(editor, listArgs, index);
            }

            ImGui::PopID();
        }
    }
}

template <typename ActionPolicy, typename EditorT>
inline void apTable_data(const std::shared_ptr<EditorT>& editor, auto... columnFunctions)
{
    auto& sel = (*editor).*(ActionPolicy::SelectionPtr);

    apTable_data_custom<ActionPolicy>(
        editor, sel.listArgs(),
        [](auto* sel, auto index) { ImGui::Selectable(sel, index); },
        columnFunctions...);
}

// ----------------

template <typename ActionPolicy, typename EditorT, size_t N_COLUMNS>
void apTable(const char* strId, const std::shared_ptr<EditorT>& editor, const std::array<const char*, N_COLUMNS>& columnNames, auto... columnFunctions)
{
    static_assert(sizeof...(columnFunctions) == N_COLUMNS);

    // Allow multiple `apTable`s on the current stack
    ImGui::PushID(strId);
    ListButtons<ActionPolicy>(editor);
    ImGui::PopID();

    if (beginApTable(strId, columnNames)) {
        apTable_data<ActionPolicy>(editor, columnFunctions...);

        endApTable();
    }
}

template <typename ActionPolicy, typename EditorT, size_t N_COLUMNS>
void apTable(const char* strId, const std::shared_ptr<EditorT>& editor, const std::array<const char*, N_COLUMNS>& columnNames,
             const ImVec2& outerSize,
             auto... columnFunctions)
{
    static_assert(sizeof...(columnFunctions) == N_COLUMNS);

    // Allow multiple `apTable`s on the current stack
    ImGui::PushID(strId);
    ListButtons<ActionPolicy>(editor);
    ImGui::PopID();

    if (beginApTable(strId, columnNames, outerSize)) {
        apTable_data<ActionPolicy>(editor, columnFunctions...);

        endApTable();
    }
}

template <typename ActionPolicy, typename EditorT, size_t N_COLUMNS>
void apTable_noScrolling(const char* strId, const std::shared_ptr<EditorT>& editor, const std::array<const char*, N_COLUMNS>& columnNames, auto... columnFunctions)
{
    static_assert(sizeof...(columnFunctions) == N_COLUMNS);

    // Allow multiple `apTable`s on the current stack
    ImGui::PushID(strId);
    ListButtons<ActionPolicy>(editor);
    ImGui::PopID();

    if (beginApTable_noScrolling(strId, columnNames)) {
        apTable_data<ActionPolicy>(editor, columnFunctions...);

        endApTable();
    }
}

}
