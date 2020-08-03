/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/abstract-editor.h"
#include "models/project/project.h"

namespace UnTech::Gui {

class ProjectSettingsEditor final : public AbstractEditor {
private:
    UnTech::Project::MemoryMapSettings _memoryMap;
    UnTech::Rooms::RoomSettings _roomSettings;

public:
    ProjectSettingsEditor(ItemIndex itemIndex);

    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    virtual void commitPendingChanges(Project::ProjectFile& projectFile) final;

    virtual void editorOpened() final;
    virtual void editorClosed() final;

    virtual void processGui(const Project::ProjectFile& projectFile) final;

private:
    void projectSettingsWindow();
};

}
