/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "project-settings-editor.h"
#include "gui/aptable.h"
#include "gui/editor-actions.h"
#include "gui/imgui-combos.h"

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

void ProjectSettingsEditorData::errorDoubleClicked(const AbstractError*)
{
    // There is no selection in ProjectSettingsEditorData.
}

void ProjectSettingsEditorData::updateSelection()
{
}

ProjectSettingsEditorGui::ProjectSettingsEditorGui()
    : AbstractEditorGui("##Project settings editor")
    , _data(nullptr)
{
}

bool ProjectSettingsEditorGui::setEditorData(AbstractEditorData* data)
{
    return (_data = dynamic_cast<ProjectSettingsEditorData*>(data));
}

void ProjectSettingsEditorGui::resetState()
{
}

void ProjectSettingsEditorGui::editorClosed()
{
}

void ProjectSettingsEditorGui::projectSettingsGui()
{
    assert(_data);
    auto& memoryMap = _data->data.memoryMap;
    auto& roomSettings = _data->data.roomSettings;

    if (ImGui::TreeNodeEx("Memory Map", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool edited = false;

        edited |= Cell("Mapping Mode", &memoryMap.mode);
        edited |= Cell_Formatted("First Bank", &memoryMap.firstBank, "0x%02X", ImGuiInputTextFlags_CharsHexadecimal);
        edited |= Cell("Number of Banks", &memoryMap.nBanks, 0);

        if (edited) {
            EditorActions<AP::MemoryMapSettings>::editorDataEdited(_data);
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("Room Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool edited = false;

        edited |= Cell_Formatted("Max Room Data Size", &roomSettings.roomDataSize, "%u bytes");

        if (edited) {
            EditorActions<AP::RoomSettings>::editorDataEdited(_data);
        }
        ImGui::TreePop();
    }
}

void ProjectSettingsEditorGui::processGui(const Project::ProjectFile&, const Project::ProjectData&)
{
    if (_data == nullptr) {
        return;
    }

    projectSettingsGui();
}

}
