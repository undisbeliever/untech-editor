/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/abstract-editor.h"
#include "gui/selection.h"
#include "gui/splitter.h"
#include "models/project/project.h"

namespace UnTech::Gui {

class ScenesEditorData final : public AbstractEditorData {
private:
    friend class ScenesEditorGui;
    struct AP;

    UnTech::Resources::ResourceScenes scenes;

    SingleSelection settingsSel;
    SingleSelection scenesSel;

public:
    explicit ScenesEditorData(ItemIndex itemIndex);

    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    virtual void errorDoubleClicked(const AbstractError*) final;
    virtual void updateSelection() final;
};

class ScenesEditorGui final : public AbstractEditorGui {
private:
    using AP = ScenesEditorData::AP;

    ScenesEditorData* _data;

    SplitterBarState _topbar;

public:
    ScenesEditorGui();

    virtual bool setEditorData(AbstractEditorData* data) final;
    virtual void resetState() final;
    virtual void editorClosed() final;

    virtual void processGui(const Project::ProjectFile& projectFile,
                            const Project::ProjectData& projectData) final;

private:
    void settingsGui();
    void scenesGui(const Project::ProjectFile& projectFile);
};

}
