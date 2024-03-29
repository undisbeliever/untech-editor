/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/aabb.h"
#include "models/common/idstring.h"
#include "models/common/iterators.h"
#include "models/common/ms8aabb.h"
#include "vendor/imgui/imgui.h"
#include <string>
#include <vector>

namespace UnTech {
struct rgba;

template <typename T>
class ExternalFileList;

template <typename T>
class NamedList;
}

namespace UnTech::Snes {
class SnesColor;
}

namespace UnTech::Gui {
class ToggleSelection;
class SingleSelection;
class MultipleSelection;
class MultipleChildSelection;
class GroupMultipleSelection;
}

// C++20 casting helpers
// =====================
inline const char* u8Cast(const char8_t* str)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<const char*>(str);
}
inline const char* u8Cast(const std::u8string& str)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<const char*>(str.c_str());
}
inline const char* u8Cast(const UnTech::idstring& str)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<const char*>(str.c_str());
}

namespace ImGui {

inline void TextUnformatted(const std::u8string_view text)
{
    TextUnformatted(u8Cast(text.data()), u8Cast(text.data() + text.size()));
}

inline void TextUnformatted(const UnTech::idstring& text)
{
    TextUnformatted(text.str());
}

inline bool ColorEdit3(const char* label, ImColor* c, ImGuiColorEditFlags flags = 0)
{
    static_assert(std::is_same_v<decltype(c->Value), ImVec4>);

    return ColorEdit3(label, &c->Value.x, flags);
}

inline bool ColorPicker3(const char* label, ImColor* c, ImGuiColorEditFlags flags = 0)
{
    static_assert(std::is_same_v<decltype(c->Value), ImVec4>);

    return ColorPicker3(label, &c->Value.x, flags);
}

bool InputText(const char* label, std::u8string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
bool InputTextMultiline(const char* label, std::u8string* str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
bool InputTextWithHint(const char* label, const char* hint, std::u8string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);

bool InputUsize(const char* label, UnTech::usize* usize, const UnTech::usize& maxSize);
bool InputUpoint(const char* label, UnTech::upoint* upoint, const UnTech::usize& containerSize);
bool InputUpoint(const char* label, UnTech::upoint* upoint, const UnTech::usize& containerSize, const UnTech::usize& itemSize);
bool InputUrect(const char* label, UnTech::urect* urect, const UnTech::usize& containerSize);

bool InputPoint(const char* label, UnTech::point* point, const UnTech::rect& bounds);
bool InputMs8rect(const char* label, UnTech::ms8rect* rect);

bool InputUnsignedFormat(const char* label, uint32_t* v, const char* format, ImGuiInputTextFlags flags = 0);

bool InputUnsigned(const char* label, uint32_t* v, unsigned step = 1, unsigned step_fast = 16, ImGuiInputTextFlags flags = 0);
bool InputUnsigned(const char* label, uint32_t* v, unsigned step, unsigned step_fast, const char* format, ImGuiInputTextFlags flags = 0);

bool InputMs8point(const char* label, UnTech::ms8point* point);
bool InputIntMs8(const char* label, UnTech::int_ms8_t* v, unsigned step = 1, unsigned step_fast = 16, ImGuiInputTextFlags flags = 0);

bool InputUint8(const char* label, uint8_t* v, unsigned step = 1, unsigned step_fast = 16, ImGuiInputTextFlags flags = 0);
bool InputUint8(const char* label, uint8_t* v, unsigned step, unsigned step_fast, const char* format, ImGuiInputTextFlags flags = 0);

bool InputUint16(const char* label, uint16_t* v, unsigned step = 1, unsigned step_fast = 16, ImGuiInputTextFlags flags = 0);
bool InputUint16(const char* label, uint16_t* v, unsigned step, unsigned step_fast, const char* format, ImGuiInputTextFlags flags = 0);

bool InputIdstring(const char* label, UnTech::idstring* idstring);

bool InputRgb(const char* label, UnTech::rgba* color, ImGuiColorEditFlags flags = 0);

bool ToggledButton(const char* label, bool selected, const ImVec2& size = ImVec2(0, 0));
bool ToggledImageButton(const char* str_id, ImTextureID user_texture_id, bool selected, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1));
bool ToggledButton(const char* label, bool* selected, const ImVec2& size = ImVec2(0, 0));

// This tree node is open by default
bool TreeNodeToggleSelection(const char* label, UnTech::Gui::SingleSelection* sel, const unsigned i);

bool Selectable(const char* label, UnTech::Gui::ToggleSelection* sel, ImGuiSelectableFlags flags = 0);
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

bool Selectable(const char* label, UnTech::Gui::GroupMultipleSelection* sel, const unsigned groupIndex, const unsigned i, ImGuiSelectableFlags flags = 0);

bool ButtonWithTooltip(const char* label, const char* tooltip, const ImVec2& size = ImVec2(0, 0));
bool ToggledButtonWithTooltip(const char* label, bool selected, const char* tooltip, const ImVec2& size = ImVec2(0, 0));
bool ToggledButtonWithTooltip(const char* label, bool* selected, const char* tooltip, const ImVec2& size = ImVec2(0, 0));

template <class T>
void NamedListListBox(const char* label, UnTech::Gui::SingleSelection* sel, const UnTech::NamedList<T>& list)
{
    if (ImGui::BeginListBox(label)) {
        for (const auto [i, item] : enumerate(list)) {
            const char* name = u8Cast(item.name);

            ImGui::PushID(i);
            ImGui::Selectable(name, sel, i);
            ImGui::PopID();
        }
        ImGui::EndListBox();
    }
}

void SameLineWithSeparator();

// These functions will append the text to the tooltip (unlike SetToolTip which overrides the previous tooltip).
void ShowTooltip(const char* text);
void ShowTooltip(const std::u8string_view text);
inline void ShowTooltip(const UnTech::idstring& text) { ShowTooltip(text.str()); };
void ShowTooltipFmt(const char* fmt, ...) IM_FMTARGS(1);

}
