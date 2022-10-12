/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/abstract-editor.h"
#include "gui/graphics/invalid-image-error-graphics.h"
#include "gui/texture.h"
#include "models/project/project.h"

namespace UnTech::Gui {

class BackgroundImageEditorData final : public AbstractEditorData {
private:
    friend class BackgroundImageEditorGui;
    struct AP;

    UnTech::Resources::BackgroundImageInput data;

public:
    explicit BackgroundImageEditorData(ItemIndex itemIndex);

    bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    void errorDoubleClicked(const AbstractError*) final;
    void updateSelection() final;
};

class BackgroundImageEditorGui final : public AbstractEditorGui {
private:
    using AP = BackgroundImageEditorData::AP;

    BackgroundImageEditorData* _data;

    InvalidImageErrorGraphics _invalidTiles;
    unsigned _invalidTilesCompileId;

    Texture _imageTexture;

public:
    bool _textureValid;

public:
    BackgroundImageEditorGui();

    bool setEditorData(AbstractEditorData* data) final;
    void resetState() final;
    void editorClosed() final;

    void processGui(const Project::ProjectFile& projectFile,
                    const Project::ProjectData& projectData) final;

private:
    void backgroundImageGui(const Project::ProjectFile& projectFile);

    void updateImageTexture();

    void updateInvalidTileList(const Project::ProjectData& projectData);
};

}
