/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "imgui-filebrowser.h"

#include "vendor/imgui-filebrowser/imfilebrowser.h"
#include "vendor/imgui/imgui_internal.h"

namespace ImGui {

static ImGuiID saveDialogId = 0;
static FileBrowser saveDialog(ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CloseOnEsc);

static ImGuiID openDialogId = 0;
static FileBrowser openDialog(ImGuiFileBrowserFlags_CloseOnEsc);

static void openSaveDialog(const ImGuiID id, const std::string& title, const char* extension)
{
    if (saveDialogId != id || !saveDialog.IsOpened()) {
        saveDialogId = id;

        saveDialog.SetTitle(title);
        saveDialog.SetTypeFilters({ extension });
        saveDialog.Open();
    }
}

static std::pair<bool, std::optional<std::filesystem::path>> processSaveDialog(const ImGuiID id, const char* extension)
{
    if (saveDialogId == id) {
        saveDialog.Display();

        if (!saveDialog.IsOpened()) {
            if (saveDialog.HasSelected()) {
                auto fn = saveDialog.GetSelected();
                saveDialog.ClearSelected();

                if (fn.extension().empty()) {
                    fn += extension;
                }

                std::error_code ec;
                fn = std::filesystem::absolute(fn, ec);
                if (!ec) {
                    return { true, fn.lexically_normal() };
                }
            }
            return { true, std::nullopt };
        }
    }
    return { false, std::nullopt };
}

static void openOpenDialog(const ImGuiID id, const std::string& title, const char* extension)
{
    if (openDialogId != id || !openDialog.IsOpened()) {
        openDialogId = id;

        openDialog.SetTitle(title);
        openDialog.SetTypeFilters({ extension });
        openDialog.Open();
    }
}

static std::pair<bool, std::optional<std::filesystem::path>> processOpenDialog(const ImGuiID id)
{
    if (openDialogId == id) {
        openDialog.Display();

        if (!openDialog.IsOpened()) {
            if (openDialog.HasSelected()) {
                auto fn = openDialog.GetSelected();
                openDialog.ClearSelected();

                std::error_code ec;
                fn = std::filesystem::absolute(fn, ec);
                if (!ec) {
                    return { true, fn.lexically_normal() };
                }
            }
            return { true, std::nullopt };
        }
    }
    return { false, std::nullopt };
}

std::pair<bool, std::optional<std::filesystem::path>> SaveFileDialog(const char* strId, const std::string& title, const char* extension)
{
    const ImGuiID id = GetID(strId);

    openSaveDialog(id, title, extension);
    return processSaveDialog(id, extension);
}

std::pair<bool, std::optional<std::filesystem::path>> OpenFileDialog(const char* strId, const std::string& title, const char* extension)
{
    const ImGuiID id = GetID(strId);

    openOpenDialog(id, title, extension);
    return processOpenDialog(id);
}

std::optional<std::filesystem::path> SaveFileDialogButton(const char* label, const std::string& title, const char* extension, const ImVec2& size)
{
    const ImGuiID id = GetID(label);

    if (Button(label, size)) {
        openSaveDialog(id, title, extension);
    }
    const auto [closed, fn] = processSaveDialog(id, extension);
    return fn;
}

std::optional<std::filesystem::path> OpenFileDialogButton(const char* label, const std::string& title, const char* extension, const ImVec2& size)
{
    const ImGuiID id = GetID(label);

    if (Button(label, size)) {
        openOpenDialog(id, title, extension);
    }
    const auto [closed, fn] = processOpenDialog(id);
    return fn;
}

bool InputPngImageFilename(const char* label, std::filesystem::path* path)
{
    const auto innerSpacingX = ImGui::GetStyle().ItemInnerSpacing.x;

    bool pathEdited = false;

    const ImGuiID id = GetID(label);

    BeginGroup();
    PushID(label);

    const float buttonWidth = CalcItemWidth();
    std::string basename = path->filename().u8string();

    if (Button(basename.data(), ImVec2(buttonWidth, 0))) {
        if (!path->empty()) {
            std::error_code ec;
            auto p = std::filesystem::absolute(path->parent_path(), ec);
            if (!ec) {
                openDialog.SetPwd(p.lexically_normal());
            }
        }
        openOpenDialog(id, "Select PNG Image", ".png");
    }

    const auto [closed, fn] = processOpenDialog(id);
    if (fn) {
        if (*path != fn) {
            *path = std::move(*fn);
            pathEdited = true;
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
