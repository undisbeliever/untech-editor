/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "imgui.h"
#include "list-helpers.h"
#include "selection.h"

#include "vendor/imgui/imgui_internal.h"

#include "models/common/clamp.h"
#include "models/common/rgba.h"
#include "models/snes/snescolor.h"
#include <algorithm>

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

bool InputUpoint(const char* label, UnTech::upoint* upoint, const UnTech::usize& containerSize)
{
    return groupedInput(
        label,
        [&]() {
            bool edited = InputUnsigned("##x", &upoint->x, 0, 0);
            if (edited) {
                if (upoint->x >= containerSize.width) {
                    upoint->x = containerSize.width > 0 ? containerSize.width - 1 : 0;
                }
            }
            return edited;
        },
        [&]() {
            bool edited = InputUnsigned("##y", &upoint->y, 0, 0);
            if (edited) {
                if (upoint->y >= containerSize.height) {
                    upoint->y = containerSize.height > 0 ? containerSize.height - 1 : 0;
                }
            }
            return edited;
        });
}

bool InputUpoint(const char* label, UnTech::upoint* upoint, const UnTech::usize& containerSize, const UnTech::usize& itemSize)
{
    return groupedInput(
        label,
        [&]() {
            bool edited = InputUnsigned("##x", &upoint->x, 0, 0);
            if (edited) {
                if (upoint->x >= containerSize.width) {
                    upoint->x = containerSize.width > itemSize.width ? containerSize.width - itemSize.width : 0;
                }
            }
            return edited;
        },
        [&]() {
            bool edited = InputUnsigned("##y", &upoint->y, 0, 0);
            if (edited) {
                if (upoint->y >= containerSize.height) {
                    upoint->y = containerSize.height > itemSize.height ? containerSize.height - itemSize.height : 0;
                }
            }
            return edited;
        });
}

bool InputUrect(const char* label, UnTech::urect* urect, const UnTech::usize& containerSize)
{
    return groupedInput(
        label,
        [&]() {
            bool edited = InputUnsigned("##x", &urect->x, 0, 0);
            if (edited) {
                unsigned maxX = (containerSize.width > urect->width) ? containerSize.width - urect->width : 0;
                if (urect->x > maxX) {
                    urect->x = maxX;
                }
            }
            return edited;
        },
        [&]() {
            bool edited = InputUnsigned("##y", &urect->y, 0, 0);
            if (edited) {
                unsigned maxY = (containerSize.height > urect->height) ? containerSize.height - urect->height : 0;
                if (urect->y > maxY) {
                    urect->y = maxY;
                }
            }
            return edited;
        },
        [&]() {
            bool edited = InputUnsigned("##width", &urect->width, 0, 0);
            if (edited) {
                unsigned maxWidth = (containerSize.width > urect->x) ? containerSize.width - urect->x : 0;
                if (urect->width > maxWidth) {
                    urect->width = maxWidth;
                }
            }
            return edited;
        },
        [&]() {
            bool edited = InputUnsigned("##height", &urect->height, 0, 0);
            if (edited) {
                unsigned maxHeight = (containerSize.height > urect->y) ? containerSize.height - urect->y : 0;
                if (urect->height > maxHeight) {
                    urect->height = maxHeight;
                }
            }
            return edited;
        });
}

bool InputPoint(const char* label, UnTech::point* point, const UnTech::rect& bounds)
{
    return groupedInput(
        label,
        [&]() {
            bool edited = InputInt("##x", &point->x, 0, 0);
            if (edited) {
                point->x = UnTech::clamp<int>(point->x, bounds.left(), bounds.right() - 1);
            }
            return edited;
        },
        [&]() {
            bool edited = InputInt("##y", &point->y, 0, 0);
            if (edited) {
                point->y = UnTech::clamp<int>(point->y, bounds.top(), bounds.bottom() - 1);
            }
            return edited;
        });
}

bool InputMs8point(const char* label, UnTech::ms8point* point)
{
    return groupedInput(
        label,
        [&]() { return InputIntMs8("##x", &point->x, 0, 0); },
        [&]() { return InputIntMs8("##y", &point->y, 0, 0); });
}

bool InputMs8rect(const char* label, UnTech::ms8rect* rect)
{
    return groupedInput(
        label,
        [&]() { return InputIntMs8("##x", &rect->x, 0, 0); },
        [&]() { return InputIntMs8("##y", &rect->y, 0, 0); },
        [&]() { return InputUint8("##width", &rect->width, 0, 0); },
        [&]() { return InputUint8("##height", &rect->height, 0, 0); });
}

bool InputUnsignedFormat(const char* label, uint32_t* v, const char* format, ImGuiInputTextFlags flags)
{
    return InputScalar(label, ImGuiDataType_U32, (void*)v, NULL, NULL, format, flags);
}

bool InputUnsigned(const char* label, uint32_t* v, unsigned step, unsigned step_fast, ImGuiInputTextFlags flags)
{
    const char* format = (flags & ImGuiInputTextFlags_CharsHexadecimal) ? "%08X" : "%d";
    return InputScalar(label, ImGuiDataType_U32, (void*)v, (void*)(step > 0 ? &step : NULL), (void*)(step_fast > 0 ? &step_fast : NULL), format, flags);
}

bool InputUnsigned(const char* label, uint32_t* v, unsigned step, unsigned step_fast, const char* format, ImGuiInputTextFlags flags)
{
    return InputScalar(label, ImGuiDataType_U32, (void*)v, (void*)(step > 0 ? &step : NULL), (void*)(step_fast > 0 ? &step_fast : NULL), format, flags);
}

bool InputIntMs8(const char* label, UnTech::int_ms8_t* v, unsigned step, unsigned step_fast, ImGuiInputTextFlags flags)
{
    int32_t i = *v;
    bool edited = InputScalar(label, ImGuiDataType_S32, &i, (void*)(step > 0 ? &step : NULL), (void*)(step_fast > 0 ? &step_fast : NULL), NULL, flags);

    if (edited) {
        *v = i;
    }
    return edited;
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

bool TreeNodeToggleSelection(const char* label, UnTech::Gui::SingleSelection* sel, const unsigned i)
{
    constexpr ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    const ImGuiTreeNodeFlags flags = sel->isSelected(i) == false ? baseFlags : baseFlags | ImGuiTreeNodeFlags_Selected;

    const bool open = ImGui::TreeNodeEx(label, flags);
    if (ImGui::IsItemClicked()) {
        sel->selectionClicked(i, true);
    }
    return open;
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

bool Selectable(const char* label, UnTech::Gui::GroupMultipleSelection* sel, const unsigned groupIndex, const unsigned i, ImGuiSelectableFlags flags)
{
    bool s = Selectable(label, sel->isSelected(groupIndex, i), flags);
    if (s) {
        sel->selectionClicked(groupIndex, i, ImGui::GetIO().KeyCtrl);
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

bool ButtonWithTooltip(const char* label, const char* tooltip, const ImVec2& size)
{
    bool pressed = ImGui::Button(label, size);
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted(tooltip);
        ImGui::EndTooltip();
    }

    return pressed;
}

bool ToggledButtonWithTooltip(const char* label, bool selected, const char* tooltip, const ImVec2& size)
{
    bool pressed = ImGui::ToggledButton(label, selected, size);
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted(tooltip);
        ImGui::EndTooltip();
    }

    return pressed;
}

bool ToggledButtonWithTooltip(const char* label, bool* selected, const char* tooltip, const ImVec2& size)
{
    bool pressed = ImGui::ToggledButton(label, selected, size);
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted(tooltip);
        ImGui::EndTooltip();
    }

    return pressed;
}

}
