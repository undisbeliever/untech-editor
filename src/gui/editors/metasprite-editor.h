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
#include "gui/texture.h"
#include "models/common/vectorset.h"
#include "models/project/project.h"

namespace UnTech::Gui {

class AabbGraphics;

class MetaSpriteEditorData final : public AbstractMetaSpriteEditorData {
private:
    friend class MetaSpriteEditorGui;
    friend class AbstractMetaSpriteEditorGui;
    struct AP;

    UnTech::MetaSprite::MetaSprite::FrameSet data;

    SingleSelection framesSel;

    ToggleSelection tileHitboxSel;
    ToggleSelection shieldSel;
    ToggleSelection hitboxSel;
    ToggleSelection hurtboxSel;

    MultipleChildSelection frameObjectsSel;
    MultipleChildSelection actionPointsSel;

    SingleSelection smallTilesetSel;
    SingleSelection largeTilesetSel;

    SingleSelection palettesSel;

    bool _tileSelectionValid;

public:
    MetaSpriteEditorData(ItemIndex itemIndex);

    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    virtual void saveFile() const final;
    virtual void errorDoubleClicked(const AbstractError*) final;
    virtual void updateSelection() final;

protected:
    void updateTileSelection();
};

class MetaSpriteEditorGui final : public AbstractMetaSpriteEditorGui {
private:
    using AP = MetaSpriteEditorData::AP;

    enum class PaletteState;
    using ObjectSize = UnTech::MetaSprite::ObjectSize;

    MetaSpriteEditorData* _data;

    unsigned _colorSel;
    PaletteState _paletteState;

    vectorset<size_t> _editedTiles;

    int _selectedEditorBgColor;

    AabbGraphics _graphics;
    Texture _paletteTexture;
    Texture _tilesetTexture;

    Image _paletteImage;
    std::unique_ptr<Image> _tilesetImage;

    ImU32 _paletteBackgroundColor;

    ImVec2 _smallTilesetUvSize;
    ImVec2 _largeTilesetUvSize;
    ImVec2 _smallTilesetUVmax;
    ImVec2 _largeTilesetUVmax;

public:
    bool _paletteValid;
    bool _tilesetValid;

    static const ImVec2 _paletteUvSize;

    static const char* colorPopupStrId;

public:
    MetaSpriteEditorGui();

    virtual bool setEditorData(AbstractEditorData* data) final;
    virtual void resetState() final;
    virtual void editorClosed() final;

    virtual void processGui(const Project::ProjectFile& projectFile,
                            const Project::ProjectData& projectData) final;

    virtual void viewMenu() final;

protected:
    virtual void addFrame(const idstring& name) final;
    virtual void addAnimation(const idstring& name) final;

private:
    void frameSetPropertiesWindow(const Project::ProjectFile& projectFile);
    void framePropertiesWindow(const Project::ProjectFile& projectFile);
    void frameContentsWindow(const Project::ProjectFile& projectFile);
    void palettesWindow();
    void colorPopup();
    void tilesetWindow();
    void frameEditorWindow();

    void drawAnimationFrame(const ImVec2& pos, const ImVec2& zoom, const UnTech::MetaSprite::MetaSprite::Frame& frame) const;

    void tilesetButtons();
    void setSelectedFrameObjectsTile(const unsigned tileId, const ObjectSize objSize);
    template <typename TilesetPolicy>
    void drawTileset(const char* label, typename TilesetPolicy::ListT* tileset, ImDrawList* drawList, const int z, const ImVec2 uv0, const ImVec2 uv1);

    ImU32 selectedPaletteBackground();

    void updatePaletteTexture();
    void updateTilesetTexture();
    void updateSelection();

    template <auto FieldPtr>
    void collisionBox(const char* label, UnTech::MetaSprite::MetaSprite::Frame& frame, ToggleSelection* sel);
};

}
