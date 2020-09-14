/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/abstract-editor.h"
#include "models/project/project.h"

namespace UnTech::Gui {

class ProjectSettingsEditorData final : public AbstractEditorData {
private:
    friend class ProjectSettingsEditorGui;
    struct AP;

    UnTech::Project::MemoryMapSettings memoryMap;
    UnTech::Rooms::RoomSettings roomSettings;

public:
    ProjectSettingsEditorData(ItemIndex itemIndex);

    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    virtual void updateSelection() final;
};

class ProjectSettingsEditorGui final : public AbstractEditorGui {
private:
    using AP = ProjectSettingsEditorData::AP;

    ProjectSettingsEditorData* _data;

public:
    ProjectSettingsEditorGui();

    virtual bool setEditorData(AbstractEditorData* data) final;
    virtual void editorDataChanged() final;

    virtual void editorOpened() final;
    virtual void editorClosed() final;

    virtual void processGui(const Project::ProjectFile& projectFile) final;

private:
    void projectSettingsWindow();
};

}
