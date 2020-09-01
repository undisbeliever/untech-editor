/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "imgui-filebrowser.h"
#include "vendor/imgui-filebrowser/imfilebrowser.h"

namespace UnTech::Gui {

std::optional<std::filesystem::path> saveFileDialogButton(const char* label, const std::string& title, const char* extension, const ImVec2& size)
{
    static ImGui::FileBrowser fileDialog(ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CloseOnEsc);

    ImGui::PushID(label);

    if (ImGui::Button(label, size)) {
        fileDialog.SetTitle(title);
        fileDialog.SetTypeFilters({ extension });
        fileDialog.Open();
    }
    fileDialog.Display();

    ImGui::PopID();

    if (fileDialog.HasSelected()) {
        auto fn = fileDialog.GetSelected();
        if (fn.extension().empty()) {
            fn += extension;
        }
        fileDialog.ClearSelected();
        return std::filesystem::absolute(fn);
    }

    return std::nullopt;
}

std::optional<std::filesystem::path> openFileDialogButton(const char* label, const std::string& title, const char* extension, const ImVec2& size)
{
    static ImGui::FileBrowser fileDialog(ImGuiFileBrowserFlags_CloseOnEsc);

    ImGui::PushID(label);

    if (ImGui::Button(label, size)) {
        fileDialog.SetTitle(title);
        fileDialog.SetTypeFilters({ extension });
        fileDialog.Open();
    }
    fileDialog.Display();

    ImGui::PopID();

    if (fileDialog.HasSelected()) {
        auto fn = fileDialog.GetSelected();
        fileDialog.ClearSelected();
        return std::filesystem::absolute(fn);
    }

    return std::nullopt;
}

}
