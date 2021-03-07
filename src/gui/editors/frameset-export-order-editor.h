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

class FrameSetExportOrderEditorData final : public AbstractExternalFileEditorData {
private:
    friend class FrameSetExportOrderEditorGui;
    struct AP;

    UnTech::MetaSprite::FrameSetExportOrder data;

    SingleSelection framesSel;
    MultipleChildSelection frameAlternativesSel;
    SingleSelection animationsSel;
    MultipleChildSelection animationAlternativesSel;

public:
    FrameSetExportOrderEditorData(ItemIndex itemIndex);
    ~FrameSetExportOrderEditorData() = default;

    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    virtual void saveFile() const final;
    virtual void updateSelection() final;
};

class FrameSetExportOrderEditorGui final : public AbstractEditorGui {
private:
    using AP = FrameSetExportOrderEditorData::AP;

    FrameSetExportOrderEditorData* _data;

public:
    FrameSetExportOrderEditorGui();

    virtual bool setEditorData(AbstractEditorData* data) final;
    virtual void editorDataChanged() final;

    virtual void editorOpened() final;
    virtual void editorClosed() final;

    virtual void processGui(const Project::ProjectFile& projectFile,
                            const Project::ProjectData& projectData) final;

private:
    void exportOrderWindow();

    template <typename ExportNameActionPolicy>
    void exportNameTree(const char* label);
};

}
