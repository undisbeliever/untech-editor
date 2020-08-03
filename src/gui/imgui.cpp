/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "imgui.h"
#include "list-helpers.h"
#include "selection.h"

#include "models/common/rgba.h"

namespace ImGui {

bool InputUnsignedFormat(const char* label, unsigned* v, const char* format, ImGuiInputTextFlags flags)
{
    return InputScalar(label, ImGuiDataType_U32, (void*)v, NULL, NULL, format, flags);
}

bool InputUnsigned(const char* label, unsigned* v, unsigned step, unsigned step_fast, ImGuiInputTextFlags flags)
{
    const char* format = (flags & ImGuiInputTextFlags_CharsHexadecimal) ? "%08X" : "%d";
    return InputScalar(label, ImGuiDataType_U32, (void*)v, (void*)(step > 0 ? &step : NULL), (void*)(step_fast > 0 ? &step_fast : NULL), format, flags);
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

bool Selectable(const char* label, UnTech::Gui::SingleSelection* sel, const unsigned i, ImGuiSelectableFlags flags)
{
    bool s = Selectable(label, (sel->selected == i), flags);
    if (s) {
        sel->clicked = i;
    }
    return s;
}

bool Selectable(UnTech::Gui::SingleSelection* sel, const unsigned i, ImGuiSelectableFlags flags)
{
    const std::string label = std::to_string(i);
    return Selectable(label.c_str(), sel, i, flags);
}

bool Selectable(const char* label, UnTech::Gui::MultipleSelection* sel, const unsigned i, ImGuiSelectableFlags flags)
{
    bool s = Selectable(label, (sel->selected & 1 << i), flags);
    if (s) {
        sel->clicked = i;
    }
    return s;
}

bool Selectable(UnTech::Gui::MultipleSelection* sel, const unsigned i, ImGuiSelectableFlags flags)
{
    const std::string label = std::to_string(i);
    return Selectable(label.c_str(), sel, i, flags);
}

bool Selectable(const char* label, UnTech::Gui::MultipleChildSelection* sel, const unsigned i, ImGuiSelectableFlags flags)
{
    bool s = Selectable(label, (sel->selected & 1 << i), flags);
    if (s) {
        sel->clicked = i;
    }
    return s;
}

bool Selectable(UnTech::Gui::MultipleChildSelection* sel, const unsigned i, ImGuiSelectableFlags flags)
{
    const std::string label = std::to_string(i);
    return Selectable(label.c_str(), sel, i, flags);
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
