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

class GameStateEditorData final : public AbstractEditorData {
private:
    friend class GameStateEditorGui;
    struct AP;

    Scripting::GameState data;

    SingleSelection flagSel;
    SingleSelection wordSel;

public:
    explicit GameStateEditorData(ItemIndex itemIndex);

    bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    void errorDoubleClicked(const AbstractError*) final;
    void updateSelection() final;
};

class GameStateEditorGui final : public AbstractEditorGui {
private:
    using AP = GameStateEditorData::AP;

    std::shared_ptr<GameStateEditorData> _data;

public:
    GameStateEditorGui();

    bool setEditorData(const std::shared_ptr<AbstractEditorData>& data) final;
    void resetState() final;
    void editorClosed() final;

    void processGui(const Project::ProjectFile& projectFile,
                    const Project::ProjectData& projectData) final;

private:
    void gameStateGui(const Project::ProjectFile& projectFile);
};

}
