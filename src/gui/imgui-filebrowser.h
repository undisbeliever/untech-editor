/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "imgui.h"
#include <filesystem>
#include <optional>

namespace ImGui {

void setFileDialogDirectory(const std::filesystem::path& dir);

// Return, { dialogClosed, selectedFilename }
std::pair<bool, std::optional<std::filesystem::path>> SaveFileDialog(const char* id, const std::u8string& title, const char8_t* extension);
std::pair<bool, std::optional<std::filesystem::path>> OpenFileDialog(const char* id, const std::u8string& title, const char8_t* extension);

std::optional<std::filesystem::path> SaveFileDialogButton(const char* label, const std::u8string& title, const char8_t* extension, const ImVec2& size = ImVec2(0, 0));
std::optional<std::filesystem::path> OpenFileDialogButton(const char* label, const std::u8string& title, const char8_t* extension, const ImVec2& size = ImVec2(0, 0));

bool InputPngImageFilename(const char* label, std::filesystem::path* path);

}
