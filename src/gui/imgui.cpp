/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "imgui.h"
#include "list-helpers.h"
#include "selection.h"

#include "vendor/imgui/imgui_internal.h"

#include "models/common/rgba.h"

namespace ImGui {

// This function is based off `ImGui::InputScalarN`
template <typename FirstFunction, typename... Functions>
static bool groupedInput(const char* label, FirstFunction firstFunction, Functions... remainingFunctions)
{
    constexpr size_t nInputs = sizeof...(Functions) + 1;
    static_assert(nInputs > 1);

    const auto innerSpacingX = GetStyle().ItemInnerSpacing.x;

    bool value_changed = false;

    BeginGroup();
    PushID(label);

    PushMultiItemsWidths(nInputs, CalcItemWidth());

    value_changed |= firstFunction();
    PopItemWidth();

    const auto l = [&](auto fn) {
        SameLine(0, innerSpacingX);
        value_changed |= fn();
        PopItemWidth();
    };
    (l(remainingFunctions), ...);

    PopID();

    const char* label_end = FindRenderedTextEnd(label);
    if (label != label_end) {
        SameLine(0.0f, innerSpacingX);
        TextEx(label, label_end);
    }

    EndGroup();
    return value_changed;
}

bool InputUsize(const char* label, UnTech::usize* usize, const UnTech::usize& maxSize)
{
    bool edited = groupedInput(
        label,
        [&]() { return InputUnsigned("##width", &usize->width, 0, 0); },
        [&]() { return InputUnsigned("##height", &usize->height, 0, 0); });

    if (edited) {
        usize->width = std::min(usize->width, maxSize.width);
        usize->height = std::min(usize->height, maxSize.height);
    }
    return edited;
}

bool InputUnsignedFormat(const char* label, unsigned* v, const char* format, ImGuiInputTextFlags flags)
{
    return InputScalar(label, ImGuiDataType_U32, (void*)v, NULL, NULL, format, flags);
}

bool InputUnsigned(const char* label, unsigned* v, unsigned step, unsigned step_fast, ImGuiInputTextFlags flags)
{
    const char* format = (flags & ImGuiInputTextFlags_CharsHexadecimal) ? "%08X" : "%d";
    return InputScalar(label, ImGuiDataType_U32, (void*)v, (void*)(step > 0 ? &step : NULL), (void*)(step_fast > 0 ? &step_fast : NULL), format, flags);
}

bool InputUnsigned(const char* label, unsigned* v, unsigned step, unsigned step_fast, const char* format, ImGuiInputTextFlags flags)
{
    return InputScalar(label, ImGuiDataType_U32, (void*)v, (void*)(step > 0 ? &step : NULL), (void*)(step_fast > 0 ? &step_fast : NULL), format, flags);
}

bool InputUint8(const char* label, uint8_t* v, unsigned step, unsigned step_fast, ImGuiInputTextFlags flags)
{
    const char* format = (flags & ImGuiInputTextFlags_CharsHexadecimal) ? "%02X" : "%d";
    return InputScalar(label, ImGuiDataType_U8, (void*)v, (void*)(step > 0 ? &step : NULL), (void*)(step_fast > 0 ? &step_fast : NULL), format, flags);
}

bool InputUint8(const char* label, uint8_t* v, unsigned step, unsigned step_fast, const char* format, ImGuiInputTextFlags flags)
{
    return InputScalar(label, ImGuiDataType_U8, (void*)v, (void*)(step > 0 ? &step : NULL), (void*)(step_fast > 0 ? &step_fast : NULL), format, flags);
}

bool InputUint16(const char* label, uint16_t* v, unsigned step, unsigned step_fast, ImGuiInputTextFlags flags)
{
    const char* format = (flags & ImGuiInputTextFlags_CharsHexadecimal) ? "%04X" : "%d";
    return InputScalar(label, ImGuiDataType_U16, (void*)v, (void*)(step > 0 ? &step : NULL), (void*)(step_fast > 0 ? &step_fast : NULL), format, flags);
}

bool InputUint16(const char* label, uint16_t* v, unsigned step, unsigned step_fast, const char* format, ImGuiInputTextFlags flags)
{
    return InputScalar(label, ImGuiDataType_U16, (void*)v, (void*)(step > 0 ? &step : NULL), (void*)(step_fast > 0 ? &step_fast : NULL), format, flags);
}

static int IdstringFilter(ImGuiInputTextCallbackData* data)
{
    if (data->EventChar == ' ') {
        data->EventChar = '_';
        return 0;
    }

    if (data->EventChar < 256 && UnTech::idstring::isCharValid((char)data->EventChar)) {
        return 0;
    }
    return 1;
}

bool InputIdstring(const char* label, UnTech::idstring* idstring)
{
    return ImGui::InputText(label, &idstring->data, ImGuiInputTextFlags_CallbackCharFilter, &IdstringFilter);
}

bool InputRgb(const char* label, UnTech::rgba* color, ImGuiColorEditFlags flags)
{
    float col[4] = { color->red / 255.0f, color->green / 255.0f, color->blue / 255.0f, 1.0f };

    bool e = ColorEdit3(label, col, flags);
    if (e) {
        color->red = col[0] * 255;
        color->green = col[1] * 255;
        color->blue = col[2] * 255;

        color->alpha = 255;
    }
    return e;
}

bool ToggledButton(const char* label, bool selected, const ImVec2& size)
{
    if (selected) {
        PushStyleColor(ImGuiCol_Button, GetStyleColorVec4(ImGuiCol_ButtonActive));
    }

    bool pressed = Button(label, size);

    if (selected) {
        PopStyleColor();
    }
    return pressed;
}

bool ToggledImageButton(ImTextureID user_texture_id, bool selected, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col)
{
    if (selected) {
        PushStyleColor(ImGuiCol_Button, GetStyleColorVec4(ImGuiCol_ButtonActive));
    }

    bool pressed = ImageButton(user_texture_id, size, uv0, uv1, frame_padding, bg_col, tint_col);

    if (selected) {
        PopStyleColor();
    }
    return pressed;
}

bool ToggledButton(const char* label, bool* selected, const ImVec2& size)
{
    bool pressed = ToggledButton(label, *selected, size);
    if (pressed) {
        *selected = !(*selected);
    }
    return pressed;
}

bool ToggledImageButton(ImTextureID user_texture_id, bool* selected, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col)
{
    bool pressed = ToggledImageButton(user_texture_id, *selected, size, uv0, uv1, frame_padding, bg_col, tint_col);
    if (*selected) {
        PopStyleColor();
    }
    return pressed;
}

template <class SelectionT>
static bool Selectable_(const char* label, SelectionT* sel, const unsigned i, ImGuiSelectableFlags flags)
{
    bool s = Selectable(label, sel->isSelected(i), flags);
    if (s) {
        sel->selectionClicked(i, ImGui::GetIO().KeyCtrl);
    }
    return s;
}

bool Selectable(const char* label, UnTech::Gui::SingleSelection* parentSel, UnTech::Gui::MultipleChildSelection* sel, const unsigned parent, const unsigned i, ImGuiSelectableFlags flags)
{
    bool s = Selectable(label, sel->isSelected(parent, i), flags);
    if (s) {
        parentSel->setSelected(parent);
        sel->selectionClicked(parent, i, ImGui::GetIO().KeyCtrl);
    }
    return s;
}

bool Selectable(const char* label, UnTech::Gui::SingleSelection* sel, const unsigned i, ImGuiSelectableFlags flags)
{
    return Selectable_(label, sel, i, flags);
}

bool Selectable(UnTech::Gui::SingleSelection* sel, const unsigned i, ImGuiSelectableFlags flags)
{
    const std::string label = std::to_string(i);
    return Selectable_(label.c_str(), sel, i, flags);
}

bool Selectable(const char* label, UnTech::Gui::MultipleSelection* sel, const unsigned i, ImGuiSelectableFlags flags)
{
    return Selectable_(label, sel, i, flags);
}

bool Selectable(UnTech::Gui::MultipleSelection* sel, const unsigned i, ImGuiSelectableFlags flags)
{
    const std::string label = std::to_string(i);
    return Selectable_(label.c_str(), sel, i, flags);
}

bool Selectable(const char* label, UnTech::Gui::MultipleChildSelection* sel, const unsigned i, ImGuiSelectableFlags flags)
{
    return Selectable_(label, sel, i, flags);
}

bool Selectable(UnTech::Gui::MultipleChildSelection* sel, const unsigned i, ImGuiSelectableFlags flags)
{
    const std::string label = std::to_string(i);
    return Selectable_(label.c_str(), sel, i, flags);
}

bool Selectable(UnTech::Gui::SingleSelection* parentSel, UnTech::Gui::MultipleChildSelection* sel, const unsigned parent, const unsigned i, ImGuiSelectableFlags flags)
{
    const std::string label = std::to_string(i);
    return Selectable(label.c_str(), parentSel, sel, parent, i, flags);
}

bool BeginCombo(const char* label, const std::string& current, ImGuiComboFlags flags)
{
    return BeginCombo(label, current.c_str(), flags);
}

bool IdStringCombo(const char* label, UnTech::idstring* value, const std::vector<UnTech::idstring>& list, bool includeBlank)
{
    return IdStringCombo(label, value, list, includeBlank,
                         [](const UnTech::idstring& name) { return &name; });
}

}
