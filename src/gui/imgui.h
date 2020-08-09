/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/idstring.h"
#include "vendor/imgui/imgui.h"
#include "vendor/imgui/misc/cpp/imgui_stdlib.h"
#include <string>
#include <vector>

namespace UnTech {
union rgba;

template <typename T>
class ExternalFileList;

template <typename T>
class NamedList;
}

namespace UnTech::Gui {
class SingleSelection;
class MultipleSelection;
class MultipleChildSelection;
}

namespace ImGui {

inline void TextUnformatted(const std::string& text)
{
    TextUnformatted(text.c_str(), text.c_str() + text.size());
}

template <typename EnumT>
bool EnumCombo(const char* label, EnumT* value, const char* const items[], int items_count, int height_in_items = -1)
{
    int v = static_cast<int>(*value);

    bool c = ImGui::Combo(label, &v, items, items_count, height_in_items);
    if (c) {
        if (*value != static_cast<EnumT>(v)) {
            *value = static_cast<EnumT>(v);
            return true;
        }
    }
    return false;
}

bool InputUnsignedFormat(const char* label, unsigned* v, const char* format, ImGuiInputTextFlags flags = 0);

bool InputUnsigned(const char* label, unsigned* v, unsigned step = 1, unsigned step_fast = 16, ImGuiInputTextFlags flags = 0);
bool InputUnsigned(const char* label, unsigned* v, unsigned step, unsigned step_fast, const char* format, ImGuiInputTextFlags flags = 0);

bool InputIdstring(const char* label, UnTech::idstring* idstring);

bool InputRgb(const char* label, UnTech::rgba* color, ImGuiColorEditFlags flags = 0);

bool Selectable(const char* label, UnTech::Gui::SingleSelection* sel, const unsigned i, ImGuiSelectableFlags flags = 0);
bool Selectable(UnTech::Gui::SingleSelection* sel, const unsigned i, ImGuiSelectableFlags flags = 0);
bool Selectable(const char* label, UnTech::Gui::MultipleSelection* sel, const unsigned i, ImGuiSelectableFlags flags = 0);
bool Selectable(UnTech::Gui::MultipleSelection* sel, const unsigned i, ImGuiSelectableFlags flags = 0);
bool Selectable(const char* label, UnTech::Gui::MultipleChildSelection* sel, const unsigned i, ImGuiSelectableFlags flags = 0);
bool Selectable(UnTech::Gui::MultipleChildSelection* sel, const unsigned i, ImGuiSelectableFlags flags = 0);

bool Selectable(const char* label, UnTech::Gui::SingleSelection* parentSel, UnTech::Gui::MultipleChildSelection* sel,
                const unsigned parent, const unsigned i, ImGuiSelectableFlags flags = 0);
bool Selectable(UnTech::Gui::SingleSelection* parentSel, UnTech::Gui::MultipleChildSelection* sel,
                const unsigned parent, const unsigned i, ImGuiSelectableFlags flags = 0);

bool BeginCombo(const char* label, const std::string& current, ImGuiComboFlags flags = 0);

bool IdStringCombo(const char* label, UnTech::idstring* value, const std::vector<UnTech::idstring>& list,
                   bool includeBlank = false);

template <class ListT, typename UnaryFunction>
bool IdStringComboSelection(UnTech::idstring* value, const ListT& list, bool includeBlank, UnaryFunction getter)
{
    bool changed = false;

    if (includeBlank) {
        const bool selected = value->isValid() == false;
        if (ImGui::Selectable("", selected)) {
            if (!selected) {
                *value = UnTech::idstring();
                changed = true;
            }
        }
    }
    for (unsigned i = 0; i < list.size(); i++) {
        const UnTech::idstring* name = getter(list.at(i));

        if (name && name->isValid()) {
            ImGui::PushID(i);

            const bool selected = (*name == *value);
            if (ImGui::Selectable(name->str().c_str(), selected)) {
                if (!selected) {
                    *value = *name;
                    changed = true;
                }
            }

            ImGui::PopID();
        }
    }

    return changed;
}

template <class ListT, typename UnaryFunction>
inline bool IdStringCombo(const char* label, UnTech::idstring* value, const ListT& list, bool includeBlank,
                          UnaryFunction getter)
{
    bool changed = false;

    if (ImGui::BeginCombo(label, value->str())) {
        changed = IdStringComboSelection(value, list, includeBlank, getter);
        ImGui::EndCombo();
    }

    return changed;
}

template <class T>
bool IdStringCombo(const char* label, UnTech::idstring* value, const UnTech::NamedList<T>& list,
                   bool includeBlank = false)
{
    return IdStringCombo(label, value, list, includeBlank,
                         [](const T& item) { return &item.name; });
}

template <class T>
bool IdStringCombo(const char* label, UnTech::idstring* value, const UnTech::ExternalFileList<T>& list,
                   bool includeBlank = false)
{
    return IdStringCombo(label, value, list, includeBlank,
                         [](const T* item) { return item ? &item->name : nullptr; });
}

}
