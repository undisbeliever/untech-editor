/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/abstract-editor.h"
#include "gui/animation-timer.h"
#include "gui/texture.h"
#include "models/project/project.h"

namespace UnTech::Gui {

class PaletteEditorData final : public AbstractEditorData {
private:
    friend class PaletteEditorGui;
    struct AP;

    UnTech::Resources::PaletteInput data;

public:
    PaletteEditorData(ItemIndex itemIndex);

    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    virtual void updateSelection() final;
};

class PaletteEditorGui final : public AbstractEditorGui {
private:
    using AP = PaletteEditorData::AP;

    PaletteEditorData* _data;

    SingleAnimationTimer _animationTimer;

    Texture _imageTexture;
    int _frameId;
    bool _textureValid;

public:
    PaletteEditorGui();

    virtual bool setEditorData(AbstractEditorData* data) final;
    virtual void editorDataChanged() final;

    virtual void editorOpened() final;
    virtual void editorClosed() final;

    virtual void processGui(const Project::ProjectFile& projectFile,
                            const Project::ProjectData& projectData) final;

private:
    void paletteWindow();

    void updateImageTexture();
};

}
