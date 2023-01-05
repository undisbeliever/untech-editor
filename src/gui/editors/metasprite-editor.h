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

    bool _tileSelectionValid{};

public:
    explicit MetaSpriteEditorData(ItemIndex itemIndex);

    bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    void saveFile() const final;
    void errorDoubleClicked(const AbstractError*) final;
    void updateSelection() final;

protected:
    void updateTileSelection();
};

class MetaSpriteEditorGui final : public AbstractMetaSpriteEditorGui {
private:
    using AP = MetaSpriteEditorData::AP;

    enum class PaletteState;
    using ObjectSize = UnTech::MetaSprite::ObjectSize;

    std::shared_ptr<MetaSpriteEditorData> _data;

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

    SplitterBarState _sidebar;
    SplitterBarState _bottombar;

public:
    bool _paletteValid;
    bool _tilesetValid;

    static const ImVec2 _paletteUvSize;

    static const char* colorPopupStrId;

public:
    MetaSpriteEditorGui();

    bool setEditorData(const std::shared_ptr<AbstractEditorData>& data) final;
    void resetState() final;
    void editorClosed() final;

    void processGui(const Project::ProjectFile& projectFile,
                    const Project::ProjectData& projectData) final;

    void processExtraWindows(const Project::ProjectFile& projectFile,
                             const Project::ProjectData& projectData) final;

    void viewMenu() final;

protected:
    void addFrame(const idstring& name) final;
    void addAnimation(const idstring& name) final;

private:
    void frameSetPropertiesGui(const Project::ProjectFile& projectFile);
    void framePropertiesGui(const Project::ProjectFile& projectFile);
    void frameContentsGui(const Project::ProjectFile& projectFile);
    void palettesGui();
    void colorPopup();
    void tilesetGui();
    void frameEditorGui();

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
