/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
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
    explicit PaletteEditorData(ItemIndex itemIndex);

    bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    void errorDoubleClicked(const AbstractError*) final;
    void updateSelection() final;
};

class PaletteEditorGui final : public AbstractEditorGui {
private:
    using AP = PaletteEditorData::AP;

    std::shared_ptr<PaletteEditorData> _data;

    SingleAnimationTimer _animationTimer;

    Texture _imageTexture;
    int _frameId;

public:
    bool _textureValid;

public:
    PaletteEditorGui();

    bool setEditorData(const std::shared_ptr<AbstractEditorData>& data) final;
    void resetState() final;
    void editorClosed() final;

    void processGui(const Project::ProjectFile& projectFile,
                    const Project::ProjectData& projectData) final;

private:
    void paletteGui();

    void updateImageTexture();
};

}
