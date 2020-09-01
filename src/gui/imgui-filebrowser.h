/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "imgui.h"
#include <filesystem>
#include <optional>

namespace UnTech::Gui {

std::optional<std::filesystem::path> saveFileDialogButton(const char* label, const std::string& title, const char* extension, const ImVec2& size = ImVec2(0, 0));
std::optional<std::filesystem::path> openFileDialogButton(const char* label, const std::string& title, const char* extension, const ImVec2& size = ImVec2(0, 0));

}
