/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/abstract-editor.h"
#include "gui/imgui.h"
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
    explicit FrameSetExportOrderEditorData(ItemIndex itemIndex);
    ~FrameSetExportOrderEditorData() override = default;

    bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    void saveFile() const final;
    void errorDoubleClicked(const AbstractError*) final;
    void updateSelection() final;
};

class FrameSetExportOrderEditorGui final : public AbstractEditorGui {
private:
    using AP = FrameSetExportOrderEditorData::AP;

    FrameSetExportOrderEditorData* _data;

public:
    FrameSetExportOrderEditorGui();

    bool setEditorData(AbstractEditorData* data) final;
    void resetState() final;
    void editorClosed() final;

    void processGui(const Project::ProjectFile& projectFile,
                    const Project::ProjectData& projectData) final;

private:
    void exportOrderGui();

    template <typename ExportNameActionPolicy>
    void exportNameTree(const char* label, const ImVec2& childSize);
};

}
