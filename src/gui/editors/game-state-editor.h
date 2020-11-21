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

class GameStateEditorData final : public AbstractEditorData {
private:
    friend class GameStateEditorGui;
    struct AP;

    Scripting::GameState data;

    SingleSelection flagSel;
    SingleSelection wordSel;

public:
    GameStateEditorData(ItemIndex itemIndex);

    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    virtual void updateSelection() final;
};

class GameStateEditorGui final : public AbstractEditorGui {
private:
    using AP = GameStateEditorData::AP;

    GameStateEditorData* _data;

public:
    GameStateEditorGui();

    virtual bool setEditorData(AbstractEditorData* data) final;
    virtual void editorDataChanged() final;

    virtual void editorOpened() final;
    virtual void editorClosed() final;

    virtual void processGui(const Project::ProjectFile& projectFile,
                            const Project::ProjectData& projectData) final;

private:
    void gameStateWindow(const Project::ProjectFile& projectFile);
};

}
