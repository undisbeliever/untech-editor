// This file is based on `imgui/misc/cpp/imgui_stdlib.cpp` from the Dear ImGui library.
// https://github.com/ocornut/imgui
// Licensed under the MIT License (MIT)
// Copyright (c) 2014-2022 Omar Cornut
// https://github.com/ocornut/imgui/blob/master/LICENSE.txt
//

// Changes made to this file include:
//   * Changing `std::string` to `std::u8string`
//   * Using `std::basic_string::data()` instead of `c_str()`
//   * Use `reinterpret_cast` for type casting
//

// dear imgui: wrappers for C++ standard library (STL) types (std::string, etc.)
// This is also an example of how you may wrap your own similar types.

// Compatibility:
// - std::string support is only guaranteed to work from C++11.
//   If you try to use it pre-C++11, please share your findings (w/ info about compiler/architecture)

// Changelog:
// - v0.10: Initial version. Added InputText() / InputTextMultiline() calls with std::string

// clang-format off

#include "imgui.h"

struct InputTextCallback_UserData_u8string
{
    std::u8string*          Str;
    ImGuiInputTextCallback  ChainCallback;
    void*                   ChainCallbackUserData;
};

static int InputTextCallback(ImGuiInputTextCallbackData* data)
{
    InputTextCallback_UserData_u8string* user_data = reinterpret_cast<InputTextCallback_UserData_u8string*>(data->UserData);
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    {
        // Resize string callback
        // If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
        std::u8string* str = user_data->Str;
        IM_ASSERT(data->Buf == reinterpret_cast<const char*>(str->data()));
        str->resize(data->BufTextLen);
        data->Buf = reinterpret_cast<char*>(str->data());
    }
    else if (user_data->ChainCallback)
    {
        // Forward to user callback, if any
        data->UserData = user_data->ChainCallbackUserData;
        return user_data->ChainCallback(data);
    }
    return 0;
}

bool ImGui::InputText(const char* label, std::u8string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallback_UserData_u8string cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return InputText(label, reinterpret_cast<char*>(str->data()), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
}

bool ImGui::InputTextMultiline(const char* label, std::u8string* str, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallback_UserData_u8string cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return InputTextMultiline(label, reinterpret_cast<char*>(str->data()), str->capacity() + 1, size, flags, InputTextCallback, &cb_user_data);
}

bool ImGui::InputTextWithHint(const char* label, const char* hint, std::u8string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallback_UserData_u8string cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return InputTextWithHint(label, hint, reinterpret_cast<char*>(str->data()), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
}
