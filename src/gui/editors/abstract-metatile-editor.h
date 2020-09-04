/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/abstract-editor.h"
#include "gui/imgui.h"
#include "gui/selection.h"
#include "models/common/vectorset-upoint.h"
#include "models/common/vectorset.h"
#include "models/project/project.h"

namespace UnTech::Gui {

class Texture;

class AbstractMetaTileEditor : public AbstractExternalFileEditor {
public:
    const static usize TILESET_TEXTURE_SIZE;

    enum class EditMode {
        None,
        SelectObjects,
        SelectTiles,
        PlaceTiles,
        // ::TODO add eraser cursor::
        // ::TODO add random cursor::
        // ::TODO add slope tiles cursor::
    };
    struct CursorState;

    struct Geometry {
        ImVec2 tileSize;
        ImVec2 mapSize;
        ImVec2 offset;
        ImVec2 zoom;

        point toTilePos(const ImVec2 globalPos) const;
        point toTilePos(const ImVec2 globalPos, const usize cursorSize) const;

        ImVec2 tilePosToVec2(const unsigned x, const unsigned y) const;
        ImVec2 tilePosToVec2(const upoint pos) const;
        ImVec2 tilePosToVec2(const point pos) const;
    };

protected:
    SingleSelection _paletteSel;
    SingleSelection _tilesetFrameSel;

    vectorset<uint8_t> _selectedTilesetTiles;

    // For MetaTile Tileset editor: selected scratchpad tiles
    // For Room editor: selected room map tiles
    upoint_vectorset _selectedTiles;

private:
    // safe - only one editor open at any given time
    static EditMode _currentEditMode;
    static CursorState _cursor;

    unsigned _tilesetIndex;
    unsigned _paletteIndex;

    bool _tilesetTextureOutOfDate;
    bool _collisionTextureOutOfDate;

public:
    AbstractMetaTileEditor(ItemIndex itemIndex);

protected:
    virtual grid<uint8_t>& map() = 0;
    virtual void mapTilesPlaced(const urect r) = 0;
    virtual void selectedTilesChanged() = 0;
    virtual void selectedTilesetTilesChanged() = 0;

    void markTexturesOutOfDate();
    void markTilesetTextureOutOfDate();
    void markCollisionTextureOutOfDate();

    void setTilesetIndex(unsigned index);
    void setPaletteIndex(unsigned index);

    // To be called at the start of `processGui`
    void updateTextures(const Project::ProjectFile& projectFile);

    // To be called in `loadDataFromProject`
    void resetState();

    virtual void editorOpened() override;
    virtual void editorClosed() override;
    virtual void updateSelection() override;

    // AutoZoom will zoom based on the width of the window
    Geometry mapGeometry(const usize size, const ImVec2 zoom);
    Geometry mapGeometryAutoZoom(const usize size);
    Geometry tilesetGeometry(const ImVec2 zoom);
    Geometry tilesetGeometryAutoZoom();

    void invisibleButton(const char* label, const Geometry& geo);

    void tilesetMinimapWindow(const char* label);
    void minimapWindow(const char* label);

    // Returns true if sel changed
    bool scratchpadMinimapWindow(const char* label, upoint_vectorset* sel, const Project::ProjectFile& projectFile);

    // The previous Dear ImGui item must be an invisible button that covers the entire map
    void drawTileset(const Geometry& geo);
    void drawAndEditMap(const Geometry& geo);

    bool selectObjectsButton();
    void editModeButtons();

    EditMode editMode() const { return _currentEditMode; }
    void setEditMode(EditMode mode);

    void setTileCursor(grid<uint16_t>&& tileCursor);
    const grid<uint16_t>& tileCursor() const;

    void commitPlacedTiles();
    void abandonPlacedTiles();

private:
    void drawTiles(const grid<uint8_t>& map, const Geometry& geo);
    void drawSelection(const upoint_vectorset& selection, const Geometry& geo);
    void drawGrid(ImDrawList* drawList, const Geometry& geo);

    void createTileCursorFromTilesetSelection();
    void createTileCursor(const grid<uint8_t>& map, const upoint_vectorset& selection);
    void enablePlaceTiles();

    void resetSelectorState();

    void processEditMode(const Geometry& geo);

    void drawCursorTiles(const grid<uint16_t>& tiles, const point& cursorPos, const Geometry& geo);
    void placeTiles(const grid<uint16_t>& tiles, const point cursorPos);

    void updateTilesetTexture(const UnTech::MetaTiles::MetaTileTilesetInput& tileset);
    void updateCollisionsTexture(const UnTech::MetaTiles::MetaTileTilesetInput& tileset);

    static Texture& tilesetTexture();
    static Texture& tilesetCollisionsTexture();
};

}
