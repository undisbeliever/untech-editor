/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "imgui-filebrowser.h"

#include "vendor/imgui-filebrowser/imfilebrowser.h"
#include "vendor/imgui/imgui_internal.h"

namespace ImGui {

std::pair<bool, std::optional<std::filesystem::path>> SaveFileDialog(const char* strId, const std::string& title, const char* extension)
{
    static ImGuiID fileDialogId = 0;
    static FileBrowser fileDialog(ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CloseOnEsc);

    const ImGuiID id = GetID(strId);

    if (fileDialogId != id || !fileDialog.IsOpened()) {
        fileDialogId = id;

        fileDialog.SetTitle(title);
        fileDialog.SetTypeFilters({ extension });
        fileDialog.Open();
    }
    fileDialog.Display();

    if (!fileDialog.IsOpened()) {
        if (fileDialog.HasSelected()) {
            auto fn = fileDialog.GetSelected();
            fileDialog.ClearSelected();

            if (fn.extension().empty()) {
                fn += extension;
            }
            return { true, std::filesystem::absolute(fn) };
        }
        else {
            return { true, std::nullopt };
        }
    }
    else {
        return { false, std::nullopt };
    }
}

std::pair<bool, std::optional<std::filesystem::path>> OpenFileDialog(const char* strId, const std::string& title, const char* extension)
{
    static ImGuiID fileDialogId = 0;
    static FileBrowser fileDialog(ImGuiFileBrowserFlags_CloseOnEsc);

    const ImGuiID id = GetID(strId);

    if (fileDialogId != id || !fileDialog.IsOpened()) {
        fileDialogId = id;

        fileDialog.SetTitle(title);
        fileDialog.SetTypeFilters({ extension });
        fileDialog.Open();
    }
    fileDialog.Display();

    if (!fileDialog.IsOpened()) {
        if (fileDialog.HasSelected()) {
            const auto fn = fileDialog.GetSelected();
            fileDialog.ClearSelected();

            return { true, std::filesystem::absolute(fn) };
        }
        else {
            return { true, std::nullopt };
        }
    }
    else {
        return { false, std::nullopt };
    }
}

std::optional<std::filesystem::path> SaveFileDialogButton(const char* label, const std::string& title, const char* extension, const ImVec2& size)
{
    static FileBrowser fileDialog(ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CloseOnEsc);

    PushID(label);

    if (Button(label, size)) {
        fileDialog.SetTitle(title);
        fileDialog.SetTypeFilters({ extension });
        fileDialog.Open();
    }
    fileDialog.Display();

    PopID();

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

std::optional<std::filesystem::path> OpenFileDialogButton(const char* label, const std::string& title, const char* extension, const ImVec2& size)
{
    static FileBrowser fileDialog(ImGuiFileBrowserFlags_CloseOnEsc);

    PushID(label);

    if (Button(label, size)) {
        fileDialog.SetTitle(title);
        fileDialog.SetTypeFilters({ extension });
        fileDialog.Open();
    }
    fileDialog.Display();

    PopID();

    if (fileDialog.HasSelected()) {
        auto fn = fileDialog.GetSelected();
        fileDialog.ClearSelected();
        return std::filesystem::absolute(fn);
    }

    return std::nullopt;
}

bool InputPngImageFilename(const char* label, std::filesystem::path* path)
{
    static ImGuiID activeDialogId;
    static ImGui::FileBrowser fileDialog = []() {
        ImGui::FileBrowser d(ImGuiFileBrowserFlags_CloseOnEsc);
        d.SetTitle("Select PNG Image");
        d.SetTypeFilters({ ".png" });
        return d;
    }();

    const auto innerSpacingX = ImGui::GetStyle().ItemInnerSpacing.x;

    bool pathEdited = false;

    const ImGuiID id = GetID(label);

    BeginGroup();
    PushID(label);

    const float buttonWidth = CalcItemWidth();
    std::string basename = path->filename().u8string();

    if (Button(basename.data(), ImVec2(buttonWidth, 0))) {
        if (!path->empty()) {
            auto p = std::filesystem::absolute(path->parent_path()).lexically_normal();
            fileDialog.SetPwd(p);
        }
        fileDialog.Open();
        activeDialogId = id;
    }

    if (activeDialogId == id) {
        fileDialog.Display();

        if (fileDialog.HasSelected()) {
            auto fn = std::filesystem::absolute(fileDialog.GetSelected()).lexically_normal();
            if (*path != fn) {
                *path = std::move(fn);
                pathEdited = true;
            }
            fileDialog.ClearSelected();
        }
    }

    const char* label_end = FindRenderedTextEnd(label);
    if (label != label_end) {
        SameLine(0.0f, innerSpacingX);
        TextEx(label, label_end);
    }

    PopID();
    EndGroup();

    return pathEdited;
}

}
