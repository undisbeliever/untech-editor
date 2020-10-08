/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstract-metasprite-editor.hpp"
#include "gui/abstract-editor.h"
#include "gui/graphics/aabb-graphics.h"
#include "gui/imgui.h"
#include "gui/selection.h"
#include "gui/texture.h"
#include "models/common/vectorset.h"
#include "models/project/project.h"

namespace UnTech::Gui {

class AabbGraphics;

class SpriteImporterEditorData final : public AbstractMetaSpriteEditorData {
private:
    friend class SpriteImporterEditorGui;
    friend class AbstractMetaSpriteEditorGui;
    struct AP;

    UnTech::MetaSprite::SpriteImporter::FrameSet data;

    SingleSelection framesSel;

    ToggleSelection tileHitboxSel;

    MultipleChildSelection frameObjectsSel;
    MultipleChildSelection actionPointsSel;
    MultipleChildSelection entityHitboxesSel;

public:
    SpriteImporterEditorData(ItemIndex itemIndex);

    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    virtual void saveFile() const final;
    virtual void updateSelection() final;
};

class SpriteImporterEditorGui final : public AbstractMetaSpriteEditorGui {
private:
    using AP = SpriteImporterEditorData::AP;

    SpriteImporterEditorData* _data;

    AabbGraphics _graphics;
    Texture _imageTexture;

    std::vector<std::pair<ImU32, std::string>> _transparentColorCombo;

    bool _imageValid;
    bool _transparentColorComboValid;

public:
    SpriteImporterEditorGui();

    virtual bool setEditorData(AbstractEditorData* data) final;
    virtual void editorDataChanged() final;

    virtual void editorOpened() final;
    virtual void editorClosed() final;

    virtual void processGui(const Project::ProjectFile& projectFile,
                            const Project::ProjectData& projectData) final;

protected:
    virtual void addFrame(const idstring& name) final;
    virtual void addAnimation(const idstring& name) final;

private:
    void frameSetPropertiesWindow(const Project::ProjectFile& projectFile);
    void framePropertiesWindow(const Project::ProjectFile& projectFile);
    void frameContentsWindow(const Project::ProjectFile& projectFile);
    void frameEditorWindow();

    void drawAnimationFrame(const ImVec2& pos, ImVec2 zoom, const UnTech::MetaSprite::SpriteImporter::Frame& frame) const;
    void drawFrame(ImDrawList* drawList, const UnTech::MetaSprite::SpriteImporter::Frame* frame);
    void drawSelectedFrame(ImDrawList* drawList, UnTech::MetaSprite::SpriteImporter::Frame* frame);

    void updateImageTexture();
    void updateTransparentColorCombo();
};

}
