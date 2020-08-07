/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "project-settings-editor.h"
#include "gui/editor-actions.h"
#include "gui/imgui.h"

namespace UnTech::Gui {

static const char* memoryMapItems[] = {
    u8"LoROM",
    u8"HiROM",
};

// ProjectSettingsEditor Action Policies
struct ProjectSettingsEditor::AP {
    struct MemoryMapSettings {
        using EditorT = ProjectSettingsEditor;
        using EditorDataT = UnTech::Project::MemoryMapSettings;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor._memoryMap;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex&)
        {
            return &projectFile.memoryMap;
        }
    };

    struct RoomSettings {
        using EditorT = ProjectSettingsEditor;
        using EditorDataT = UnTech::Rooms::RoomSettings;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor._roomSettings;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex&)
        {
            return &projectFile.roomSettings;
        }
    };
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
                EditorActions<AP::MemoryMapSettings>::editorDataEdited(this);
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Room Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            bool edited = false;

            ImGui::InputUnsignedFormat("Max Room Data Size", &_roomSettings.roomDataSize, "%u bytes");
            edited |= ImGui::IsItemDeactivatedAfterEdit();

            if (edited) {
                EditorActions<AP::RoomSettings>::editorDataEdited(this);
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

void ProjectSettingsEditor::updateSelection()
{
}

}
