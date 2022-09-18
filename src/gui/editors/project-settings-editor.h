/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
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

    UnTech::Project::ProjectSettings data;

public:
    explicit ProjectSettingsEditorData(ItemIndex itemIndex);

    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    virtual void errorDoubleClicked(const AbstractError*) final;
    virtual void updateSelection() final;
};

class ProjectSettingsEditorGui final : public AbstractEditorGui {
private:
    using AP = ProjectSettingsEditorData::AP;

    ProjectSettingsEditorData* _data;

public:
    ProjectSettingsEditorGui();

    virtual bool setEditorData(AbstractEditorData* data) final;
    virtual void resetState() final;
    virtual void editorClosed() final;

    virtual void processGui(const Project::ProjectFile& projectFile,
                            const Project::ProjectData& projectData) final;

private:
    void projectSettingsWindow();
};

}
