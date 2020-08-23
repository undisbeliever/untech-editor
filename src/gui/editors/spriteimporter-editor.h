/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstract-metasprite-editor.hpp"
#include "gui/abstract-editor.h"
#include "gui/imgui.h"
#include "gui/selection.h"
#include "gui/texture.h"
#include "models/common/vectorset.h"
#include "models/project/project.h"

namespace UnTech::Gui {

class AabbGraphics;

class SpriteImporterEditor final : public AbstractMetaSpriteEditor {
private:
    struct AP;

    UnTech::MetaSprite::SpriteImporter::FrameSet _data;

    SingleSelection _framesSel;

    ToggleSelection _tileHitboxSel;

    MultipleChildSelection _frameObjectsSel;
    MultipleChildSelection _actionPointsSel;
    MultipleChildSelection _entityHitboxesSel;

    static std::vector<std::pair<ImU32, std::string>> _transparentColorCombo;

    static AabbGraphics _graphics;

    static bool _imageValid;
    static bool _transparentColorComboValid;

public:
    SpriteImporterEditor(ItemIndex itemIndex);

    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) final;

    virtual void editorOpened() final;
    virtual void editorClosed() final;

    virtual void processGui(const Project::ProjectFile& projectFile) final;
    virtual void updateSelection() final;

private:
    void frameSetPropertiesWindow(const Project::ProjectFile& projectFile);
    void framePropertiesWindow(const Project::ProjectFile& projectFile);
    void frameContentsWindow(const Project::ProjectFile& projectFile);
    void frameEditorWindow();

    Texture& imageTexture();

    void drawFrame(ImDrawList* drawList, const UnTech::MetaSprite::SpriteImporter::Frame* frame);
    void drawSelectedFrame(ImDrawList* drawList, UnTech::MetaSprite::SpriteImporter::Frame* frame);

    void updateImageTexture();
    void updateTransparentColorCombo();
};

}
