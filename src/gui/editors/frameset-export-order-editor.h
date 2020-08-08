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

class FrameSetExportOrderEditor final : public AbstractEditor {
private:
    struct AP;

    UnTech::MetaSprite::FrameSetExportOrder _data;

    SingleSelection _framesSel;
    MultipleChildSelection _frameAlternativesSel;
    SingleSelection _animationsSel;
    MultipleChildSelection _animationAlternativesSel;

public:
    FrameSetExportOrderEditor(ItemIndex itemIndex);
    ~FrameSetExportOrderEditor() = default;

    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) final;

    virtual void editorOpened() final;
    virtual void editorClosed() final;

    virtual void processGui(const Project::ProjectFile& projectFile) final;
    virtual void updateSelection() final;

private:
    void exportOrderWindow();

    template <typename ExportNameActionPolicy>
    void exportNameTree(const char* label);
};

}
