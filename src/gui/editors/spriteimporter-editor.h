/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstract-metasprite-editor.hpp"
#include "gui/abstract-editor.h"
#include "gui/graphics/aabb-graphics.h"
#include "gui/imgui.h"
#include "gui/selection.h"
#include "gui/splitter.h"
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
    ToggleSelection shieldSel;
    ToggleSelection hitboxSel;
    ToggleSelection hurtboxSel;

    MultipleChildSelection frameObjectsSel;
    MultipleChildSelection actionPointsSel;

public:
    explicit SpriteImporterEditorData(ItemIndex itemIndex);

    bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    void saveFile() const final;
    void errorDoubleClicked(const AbstractError*) final;
    void updateSelection() final;
};

class SpriteImporterEditorGui final : public AbstractMetaSpriteEditorGui {
private:
    using AP = SpriteImporterEditorData::AP;

    SpriteImporterEditorData* _data;

    AabbGraphics _graphics;
    Texture _imageTexture;

    std::vector<std::pair<ImU32, std::u8string>> _transparentColorCombo;

    SplitterBarState _sidebar;

public:
    bool _imageValid;
    bool _transparentColorComboValid;

public:
    SpriteImporterEditorGui();

    bool setEditorData(AbstractEditorData* data) final;
    void resetState() final;
    void editorClosed() final;

    void processGui(const Project::ProjectFile& projectFile,
                    const Project::ProjectData& projectData) final;

    void processExtraWindows(const Project::ProjectFile& projectFile,
                             const Project::ProjectData& projectData) final;

protected:
    void addFrame(const idstring& name) final;
    void addAnimation(const idstring& name) final;

private:
    void frameSetPropertiesGui(const Project::ProjectFile& projectFile);
    void framePropertiesGui(const Project::ProjectFile& projectFile);
    void frameContentsWindow(const Project::ProjectFile& projectFile);
    void frameEditorGui();

    void drawAnimationFrame(const ImVec2& drawPos, ImVec2 zoom, const UnTech::MetaSprite::SpriteImporter::Frame& frame) const;
    void drawFrame(ImDrawList* drawList, const UnTech::MetaSprite::SpriteImporter::Frame* frame);
    void drawSelectedFrame(ImDrawList* drawList, UnTech::MetaSprite::SpriteImporter::Frame* frame);

    void updateImageTexture();
    void updateTransparentColorCombo();

    template <auto FieldPtr>
    void collisionBox(const char* label, UnTech::MetaSprite::SpriteImporter::Frame& frame, const usize& frameSize, ToggleSelection* sel);
};

}
