/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/abstract-editor.h"
#include "gui/animation-timer.h"
#include "gui/imgui.h"
#include "gui/selection.h"
#include "gui/shaders.h"
#include "models/common/vectorset-upoint.h"
#include "models/common/vectorset.h"
#include "models/project/project.h"

namespace UnTech::Gui {

class AbstractMetaTileEditorData : public AbstractExternalFileEditorData {
public:
    friend class AbstractMetaTileEditorGui;

protected:
    vectorset<uint8_t> selectedTilesetTiles;

    // For MetaTile Tileset editor: selected scratchpad tiles
    // For Room editor: selected room map tiles
    upoint_vectorset selectedTiles;

public:
    AbstractMetaTileEditorData(ItemIndex itemIndex);

protected:
    virtual grid<uint8_t>& map() = 0;
    virtual void mapTilesPlaced(const urect r) = 0;

    virtual void selectedTilesChanged() = 0;
    virtual void selectedTilesetTilesChanged() = 0;
};

class AbstractMetaTileEditorGui : public AbstractEditorGui {
    const static usize TILESET_TEXTURE_SIZE;

public:
    enum class EditMode {
        None,
        SelectObjects,
        SelectTiles,
        PlaceTiles,
        // ::TODO add eraser cursor::
        // ::TODO add random cursor::
        // ::TODO add slope tiles cursor::
    };

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

    struct CursorState {
        // Values greater than UINT8_MAX are transparent
        grid<uint16_t> tiles;

        point lastTilePos;

        urect modifiedTiles;

        // Tiles drawn by the editor but not committed yet
        bool mapDirty = false;
        bool currentlyEditing = false;
    };

private:
    AbstractMetaTileEditorData* _data;

protected:
    Shaders::MtTileset _tilesetShader;

private:
    Shaders::MtTilemap _tilemap;

    EditMode _currentEditMode;
    CursorState _cursor;

    DualAnimationTimer _animationTimer;

    bool _tilemapOutOfDate;

    static bool showGrid;
    static bool showTiles;
    static bool showTileCollisions;
    static bool showInteractiveTiles;

public:
    AbstractMetaTileEditorGui();

    virtual bool setEditorData(AbstractEditorData* data) override;
    virtual void editorDataChanged() override;

    virtual void editorOpened() override;
    virtual void editorClosed() override;

    virtual void viewMenu() override;

protected:
    virtual void selectionChanged() = 0;
    virtual const std::array<idstring, 256>& tileFunctionTables() const = 0;

    void showLayerButtons();

    void markTilemapOutOfDate();

    // To be called in `loadDataFromProject`
    void resetState();

    // To be called at the start of `processGui`
    void updateMapAndProcessAnimations();

    // AutoZoom will zoom based on the width of the window
    Geometry mapGeometry(const usize size, const ImVec2 zoom);
    Geometry mapGeometryAutoZoom(const usize size);
    Geometry tilesetGeometry(const ImVec2 zoom);
    Geometry tilesetGeometryAutoZoom();

    void invisibleButton(const char* label, const Geometry& geo);

    void tilesetMinimapWindow(const char* label);

    void minimapWindow(const char* label);

    // Returns true if sel changed
    bool scratchpadMinimapWindow(const char* label, const Shaders::MtTilemap& tilemap,
                                 const grid<uint8_t>& mapData, upoint_vectorset* sel);

    // The previous Dear ImGui item must be an invisible button that covers the entire map
    void drawTileset(const Geometry& geo);
    void drawAndEditMap(const Geometry& geo);

    void animationButtons();
    bool selectObjectsButton();
    void editModeButtons();

    EditMode editMode() const { return _currentEditMode; }
    void setEditMode(EditMode mode);

    void setTileCursor(grid<uint16_t>&& tileCursor);
    const grid<uint16_t>& tileCursor() const;

    void commitPlacedTiles();
    void abandonPlacedTiles();

private:
    void drawTilemap(const Shaders::MtTilemap& tilemap, const Geometry& geo);
    void drawSelection(const upoint_vectorset& selection, const Geometry& geo);
    void drawGrid(ImDrawList* drawList, const Geometry& geo);

    void tilesetInteractiveTilesTooltip(const Geometry& geo);
    void interactiveTilesTooltip(const grid<uint8_t>& mapData, const Geometry& geo);

    void createTileCursorFromTilesetSelection();
    void createTileCursor(const grid<uint8_t>& map, const upoint_vectorset& selection);
    void createTileCursorFromScratchpad(const grid<uint8_t>& map, const upoint_vectorset& selection);
    void enablePlaceTiles();

    void resetSelectorState();

    void processEditMode(const Geometry& geo);

    void drawCursorTiles(const grid<uint16_t>& tiles, const point& cursorPos, const Geometry& geo);
    void placeTiles(const grid<uint16_t>& tiles, const point cursorPos);
};

}
