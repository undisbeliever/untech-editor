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

class BytecodeEditorData final : public AbstractEditorData {
private:
    friend class BytecodeEditorGui;
    struct AP;

    Scripting::BytecodeInput data;

    SingleSelection instructionSel;

public:
    BytecodeEditorData(ItemIndex itemIndex);

    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    virtual void updateSelection() final;
};

class BytecodeEditorGui final : public AbstractEditorGui {
private:
    using AP = BytecodeEditorData::AP;

    BytecodeEditorData* _data;

public:
    BytecodeEditorGui();

    virtual bool setEditorData(AbstractEditorData* data) final;
    virtual void resetState() final;
    virtual void editorClosed() final;

    virtual void processGui(const Project::ProjectFile& projectFile,
                            const Project::ProjectData& projectData) final;

private:
    void instructionsWindow();
};

}
