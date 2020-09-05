/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "imgui.h"
#include <filesystem>
#include <optional>

namespace ImGui {

std::optional<std::filesystem::path> SaveFileDialogButton(const char* label, const std::string& title, const char* extension, const ImVec2& size = ImVec2(0, 0));
std::optional<std::filesystem::path> OpenFileDialogButton(const char* label, const std::string& title, const char* extension, const ImVec2& size = ImVec2(0, 0));

bool InputPngImageFilename(const char* label, std::filesystem::path* path);

}
