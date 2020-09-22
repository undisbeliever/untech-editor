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

class MetaSpriteEditorData final : public AbstractMetaSpriteEditorData {
private:
    friend class MetaSpriteEditorGui;
    struct AP;

    UnTech::MetaSprite::MetaSprite::FrameSet data;

    // ::TODO SingleSelection that can never be cleared::
    SingleSelection framesSel;

    ToggleSelection tileHitboxSel;

    MultipleChildSelection frameObjectsSel;
    MultipleChildSelection actionPointsSel;
    MultipleChildSelection entityHitboxesSel;

    SingleSelection smallTilesetSel;
    SingleSelection largeTilesetSel;

    SingleSelection palettesSel;

    bool _tileSelectionValid;

public:
    MetaSpriteEditorData(ItemIndex itemIndex);

    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    virtual void saveFile() const final;
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

    // ::TODO replace with 256 bit bitfield::
    vectorset<size_t> _editedTiles;

    int _selectedEditorBgColor;

    AabbGraphics _graphics;
    Texture _paletteTexture;
    Texture _tilesetTexture;

    Image _paletteImage;
    Image _tilesetImage;
    ImU32 _paletteBackgroundColor;

    ImVec2 _paletteUvSize;
    ImVec2 _smallTilesetUvSize;
    ImVec2 _largeTilesetUvSize;
    ImVec2 _smallTilesetUVmax;
    ImVec2 _largeTilesetUVmax;

    bool _paletteValid;
    bool _tilesetValid;

    static const char* colorPopupStrId;

public:
    MetaSpriteEditorGui();

    virtual bool setEditorData(AbstractEditorData* data) final;
    virtual void editorDataChanged() final;

    virtual void editorOpened() final;
    virtual void editorClosed() final;

    virtual void processGui(const Project::ProjectFile& projectFile) final;

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

    void tilesetButtons();
    void setSelectedFrameObjectsTile(const unsigned tileId, const ObjectSize objSize);
    template <typename TilesetPolicy>
    void drawTileset(const char* label, typename TilesetPolicy::ListT* tileset, ImDrawList* drawList, const int z, const ImVec2 uv0, const ImVec2 uv1);

    ImU32 selectedPaletteBackground();

    void updatePaletteTexture();
    void updateTilesetTexture();
    void updateSelection();
};

}
