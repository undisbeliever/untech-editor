/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/abstract-editor.h"
#include "gui/selection.h"
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
    ScenesEditorData(ItemIndex itemIndex);

    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    virtual void updateSelection() final;
};

class ScenesEditorGui final : public AbstractEditorGui {
private:
    using AP = ScenesEditorData::AP;

    ScenesEditorData* _data;

public:
    ScenesEditorGui();

    virtual bool setEditorData(AbstractEditorData* data) final;
    virtual void editorDataChanged() final;

    virtual void editorOpened() final;
    virtual void editorClosed() final;

    virtual void processGui(const Project::ProjectFile& projectFile) final;

private:
    void settingsWindow();
    void scenesWindow(const Project::ProjectFile& projectFile);

    bool sceneLayerCombo(const char* label, idstring* value,
                         const Project::ProjectFile& projectFile, const Resources::SceneSettingsInput& sceneSettings, const unsigned layerId);
};

}
