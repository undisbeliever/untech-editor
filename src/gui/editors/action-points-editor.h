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

class ActionPointsEditorData final : public AbstractEditorData {
private:
    friend class ActionPointsEditorGui;
    struct AP;

    NamedList<UnTech::MetaSprite::ActionPointFunction> actionPointFunctions;

    SingleSelection sel;

public:
    ActionPointsEditorData(ItemIndex itemIndex);

    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    virtual void updateSelection() final;
};

class ActionPointsEditorGui final : public AbstractEditorGui {
private:
    using AP = ActionPointsEditorData::AP;

    ActionPointsEditorData* _data;

public:
    ActionPointsEditorGui();

    virtual bool setEditorData(AbstractEditorData* data) final;
    virtual void editorDataChanged() final;

    virtual void editorOpened() final;
    virtual void editorClosed() final;

    virtual void processGui(const Project::ProjectFile& projectFile) final;

private:
    void actionPointsWindow();
};

}
