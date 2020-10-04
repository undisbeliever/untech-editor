/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "project-settings-editor.h"
#include "gui/editor-actions.h"
#include "gui/imgui-combos.h"
#include "gui/imgui.h"

namespace UnTech::Gui {

// ProjectSettingsEditor Action Policies
struct ProjectSettingsEditorData::AP {
    struct MemoryMapSettings {
        using EditorT = ProjectSettingsEditorData;
        using EditorDataT = UnTech::Project::MemoryMapSettings;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor.data.memoryMap;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex&)
        {
            return &projectFile.projectSettings.memoryMap;
        }
    };

    struct RoomSettings {
        using EditorT = ProjectSettingsEditorData;
        using EditorDataT = UnTech::Rooms::RoomSettings;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor.data.roomSettings;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex&)
        {
            return &projectFile.projectSettings.roomSettings;
        }
    };
};

ProjectSettingsEditorData::ProjectSettingsEditorData(ItemIndex itemIndex)
    : AbstractEditorData(itemIndex)
{
}

bool ProjectSettingsEditorData::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    data = projectFile.projectSettings;

    return true;
}

void ProjectSettingsEditorData::updateSelection()
{
}

ProjectSettingsEditorGui::ProjectSettingsEditorGui()
    : AbstractEditorGui()
    , _data(nullptr)
{
}

bool ProjectSettingsEditorGui::setEditorData(AbstractEditorData* data)
{
    return (_data = dynamic_cast<ProjectSettingsEditorData*>(data));
}

void ProjectSettingsEditorGui::editorDataChanged()
{
}

void ProjectSettingsEditorGui::editorOpened()
{
}

void ProjectSettingsEditorGui::editorClosed()
{
}

void ProjectSettingsEditorGui::projectSettingsWindow()
{
    assert(_data);
    auto& memoryMap = _data->data.memoryMap;
    auto& roomSettings = _data->data.roomSettings;

    if (ImGui::Begin("Project Settings")) {
        ImGui::SetWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);

        if (ImGui::TreeNodeEx("Memory Map", ImGuiTreeNodeFlags_DefaultOpen)) {
            bool edited = false;

            edited |= ImGui::EnumCombo("Mapping Mode", &memoryMap.mode);
            ImGui::IsItemDeactivatedAfterEdit();

            ImGui::InputUnsignedFormat("First Bank", &memoryMap.firstBank, "0x%02X", ImGuiInputTextFlags_CharsHexadecimal);
            edited |= ImGui::IsItemDeactivatedAfterEdit();

            ImGui::InputUnsigned("Number of Banks", &memoryMap.nBanks, 0);
            edited |= ImGui::IsItemDeactivatedAfterEdit();

            if (edited) {
                EditorActions<AP::MemoryMapSettings>::editorDataEdited(_data);
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Room Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            bool edited = false;

            ImGui::InputUnsignedFormat("Max Room Data Size", &roomSettings.roomDataSize, "%u bytes");
            edited |= ImGui::IsItemDeactivatedAfterEdit();

            if (edited) {
                EditorActions<AP::RoomSettings>::editorDataEdited(_data);
            }
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void ProjectSettingsEditorGui::processGui(const Project::ProjectFile&)
{
    if (_data == nullptr) {
        return;
    }

    projectSettingsWindow();
}

}
