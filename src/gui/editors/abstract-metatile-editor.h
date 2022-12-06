/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
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

class AabbGraphics;

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
};

class AbstractMetaTileEditorGui : public AbstractEditorGui {
    const static usize TILESET_TEXTURE_SIZE;

public:
    struct Geometry;

    enum class EditMode {
        None,
        SelectObjects,
        SelectTiles,
        PlaceTiles,
        // ::TODO add eraser cursor::
        // ::TODO add random cursor::
        // ::TODO add slope tiles cursor::
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

public:
    bool _tilemapValid;

    static bool showGrid;
    static bool showTiles;
    static bool showTileCollisions;
    static bool showInteractiveTiles;

public:
    AbstractMetaTileEditorGui(const char* strId);

    bool setEditorData(AbstractEditorData* data) override;
    void resetState() override;
    void editorClosed() override;

    void viewMenu() override;

protected:
    virtual void selectionChanged() = 0;
    [[nodiscard]] virtual const std::array<idstring, 256>& tileFunctionTables() const = 0;

    // NOTE: AbstractMetaTileEditorGui will edit the map data returned by this function.
    [[nodiscard]] virtual grid<uint8_t>& map() = 0;

    virtual void mapTilesPlaced(const urect r) = 0;

    virtual void selectedTilesChanged() = 0;
    virtual void selectedTilesetTilesChanged() = 0;

    void showLayerButtons();

    // To be called at the start of `processGui`
    void updateMapAndProcessAnimations();

    void tilesetMinimapGui(const char* label);

    void minimapGui(const char* label);

    // Returns true if sel changed
    bool scratchpadMinimapGui(const char* label, const Shaders::MtTilemap& tilemap,
                              const grid<uint8_t>& mapData, upoint_vectorset* sel);

    // These functions with a `strId` argument will create an InvisibleButton that covers the entire map.
    // Return the screen position of the tileset/map
    ImVec2 drawTileset(const char* strId, const ImVec2 zoom);
    ImVec2 drawAndEditMap(const char* strId, const ImVec2 zoom);
    ImVec2 drawAndEditMap(const AabbGraphics& graphics);

    void animationButtons();
    bool selectObjectsButton();
    void editModeButtons();

    [[nodiscard]] EditMode editMode() const { return _currentEditMode; }
    void setEditMode(EditMode mode);

    void setTileCursor(grid<uint16_t>&& tileCursor);
    [[nodiscard]] const grid<uint16_t>& tileCursor() const;

    void commitPlacedTiles();
    void abandonPlacedTiles();

private:
    void drawAndEditMap(const Geometry& geo);

    void drawTileset(const Geometry& geo);
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
