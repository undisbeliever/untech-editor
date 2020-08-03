/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "project-settings-editor.h"
#include "gui/imgui.h"

namespace UnTech::Gui {

static const char* memoryMapItems[] = {
    u8"LoROM",
    u8"HiROM",
};

ProjectSettingsEditor::ProjectSettingsEditor(ItemIndex itemIndex)
    : AbstractEditor(itemIndex)
{
}

bool ProjectSettingsEditor::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    _memoryMap = projectFile.memoryMap;
    _roomSettings = projectFile.roomSettings;

    return true;
}

void ProjectSettingsEditor::commitPendingChanges(Project::ProjectFile& projectFile)
{
    projectFile.memoryMap = _memoryMap;
    projectFile.roomSettings = _roomSettings;
}

void ProjectSettingsEditor::editorOpened()
{
}

void ProjectSettingsEditor::editorClosed()
{
}

void ProjectSettingsEditor::projectSettingsWindow()
{
    if (ImGui::Begin("Project Settings")) {
        ImGui::SetWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);

        if (ImGui::TreeNodeEx("Memory Map", ImGuiTreeNodeFlags_DefaultOpen)) {
            bool edited = false;

            edited |= ImGui::EnumCombo("Mapping Mode", &_memoryMap.mode, memoryMapItems, IM_ARRAYSIZE(memoryMapItems));
            ImGui::IsItemDeactivatedAfterEdit();

            ImGui::InputUnsignedFormat("First Bank", &_memoryMap.firstBank, "0x%02X", ImGuiInputTextFlags_CharsHexadecimal);
            edited |= ImGui::IsItemDeactivatedAfterEdit();

            ImGui::InputUnsigned("Number of Banks", &_memoryMap.nBanks, 0);
            edited |= ImGui::IsItemDeactivatedAfterEdit();

            if (edited) {
                ImGui::LogText("Edited Memory Map");
                this->pendingChanges = true;
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Room Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            bool edited = false;

            ImGui::InputUnsignedFormat("Max Room Data Size", &_roomSettings.roomDataSize, "%u bytes");
            edited |= ImGui::IsItemDeactivatedAfterEdit();

            if (edited) {
                ImGui::LogText("Edited Room Settings");
                this->pendingChanges = true;
            }
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void ProjectSettingsEditor::processGui(const Project::ProjectFile&)
{
    projectSettingsWindow();
}

}
