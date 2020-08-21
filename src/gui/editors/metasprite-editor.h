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

class MetaSpriteEditor final : public AbstractMetaSpriteEditor {
private:
    struct AP;

    enum class PaletteState;
    using ObjectSize = UnTech::MetaSprite::ObjectSize;

    UnTech::MetaSprite::MetaSprite::FrameSet _data;

    // ::TODO SingleSelection that can never be cleared::
    SingleSelection _framesSel;

    ToggleSelection _tileHitboxSel;

    MultipleChildSelection _frameObjectsSel;
    MultipleChildSelection _actionPointsSel;
    MultipleChildSelection _entityHitboxesSel;

    SingleSelection _smallTilesetSel;
    SingleSelection _largeTilesetSel;

    SingleSelection _palettesSel;
    static unsigned _colorSel;
    static PaletteState _paletteState;

    // ::TODO replace with 256 bit bitfield::
    static vectorset<size_t> _editedTiles;

    int _selectedEditorBgColor;

    static const char* colorPopupStrId;

    static AabbGraphics _graphics;

    static Image _paletteImage;
    static Image _tilesetImage;
    static ImU32 _paletteBackgroundColor;

    static ImVec2 _paletteUvSize;
    static ImVec2 _smallTilesetUvSize;
    static ImVec2 _largeTilesetUvSize;
    static ImVec2 _smallTilesetUVmax;
    static ImVec2 _largeTilesetUVmax;

    static bool _paletteValid;
    static bool _tilesetValid;
    static bool _tileSelectionValid;

public:
    MetaSpriteEditor(ItemIndex itemIndex);

    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) final;

    virtual void editorOpened() final;
    virtual void editorClosed() final;

    virtual void processGui(const Project::ProjectFile& projectFile) final;
    virtual void updateSelection() final;

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

    Texture& paletteTexture();
    Texture& tilesetTexture();

    void updatePaletteTexture();
    void updateTilesetTexture();
    void updateTileSelection();
};

}
