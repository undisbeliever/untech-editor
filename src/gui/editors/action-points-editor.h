/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
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
    explicit ActionPointsEditorData(ItemIndex itemIndex);

    bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    void errorDoubleClicked(const AbstractError*) final;
    void updateSelection() final;
};

class ActionPointsEditorGui final : public AbstractEditorGui {
private:
    using AP = ActionPointsEditorData::AP;

    std::shared_ptr<ActionPointsEditorData> _data;

public:
    ActionPointsEditorGui();

    bool setEditorData(std::shared_ptr<AbstractEditorData> data) final;
    void resetState() final;
    void editorClosed() final;

    void processGui(const Project::ProjectFile& projectFile,
                    const Project::ProjectData& projectData) final;

private:
    void actionPointsGui();
};

}
