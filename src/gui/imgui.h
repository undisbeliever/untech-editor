/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/misc/cpp/imgui_stdlib.h"
#include <string>

namespace ImGui {

inline void TextUnformatted(const std::string& text)
{
    TextUnformatted(text.c_str(), text.c_str() + text.size());
}

}
