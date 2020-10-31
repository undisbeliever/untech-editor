/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstract-metatile-editor.h"
#include "gui/graphics/tilecollisionimage.h"
#include "gui/imgui-drawing.h"
#include "gui/imgui.h"
#include "gui/style.h"
#include "gui/texture.h"
#include "models/common/vectorset-upoint.h"
#include "models/project/project-data.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace UnTech::Gui {

static constexpr unsigned METATILE_SIZE_PX = MetaTiles::METATILE_SIZE_PX;
static constexpr unsigned N_METATILES = MetaTiles::N_METATILES;
static constexpr unsigned TILESET_WIDTH = MetaTiles::TILESET_WIDTH;
static constexpr unsigned TILESET_HEIGHT = MetaTiles::TILESET_HEIGHT;

// ::TODO add animation region to view menu::
// ::TODO add to View Menu::
bool AbstractMetaTileEditorGui::showGrid = true;
bool AbstractMetaTileEditorGui::showTiles = true;
bool AbstractMetaTileEditorGui::showTileCollisions = true;
bool AbstractMetaTileEditorGui::showInteractiveTiles = true;

const usize AbstractMetaTileEditorGui::TILESET_TEXTURE_SIZE{
    TILESET_WIDTH * METATILE_SIZE_PX,
    TILESET_HEIGHT* METATILE_SIZE_PX,
};

point AbstractMetaTileEditorGui::Geometry::toTilePos(const ImVec2 globalPos) const
{
    const ImVec2 mousePos = ImVec2(globalPos.x - offset.x, globalPos.y - offset.y);
    return point(std::floor(mousePos.x / tileSize.x), std::floor(mousePos.y / tileSize.y));
}

point AbstractMetaTileEditorGui::Geometry::toTilePos(const ImVec2 globalPos, const usize cursorSize) const
{
    const point p = toTilePos(globalPos);
    return point(p.x - int(cursorSize.width / 2), p.y - int(cursorSize.height / 2));
}

ImVec2 AbstractMetaTileEditorGui::Geometry::tilePosToVec2(const unsigned x, const unsigned y) const
{
    return ImVec2(offset.x + tileSize.x * x,
                  offset.y + tileSize.y * y);
}

ImVec2 AbstractMetaTileEditorGui::Geometry::tilePosToVec2(const upoint pos) const
{
    return tilePosToVec2(pos.x, pos.y);
}

ImVec2 AbstractMetaTileEditorGui::Geometry::tilePosToVec2(const point pos) const
{
    return ImVec2(offset.x + tileSize.x * pos.x,
                  offset.y + tileSize.y * pos.y);
}

template <typename SelectionT>
class TileSelector {
private:
    usize _mapSize;

    SelectionT _previousSelection;
    upoint _previousTilePos;
    upoint _boxSelectionStart;
    bool _previousTileSelected = false;
    bool _draging = false;
    bool _clickedPosValid = false;

    upoint clampPoint(const point& p)
    {
        return upoint{
            unsigned(std::clamp<int>(p.x, 0, _mapSize.width - 1)),
            unsigned(std::clamp<int>(p.y, 0, _mapSize.height - 1))
        };
    }

    upoint clampPoint(const upoint& p)
    {
        return upoint{
            std::min<unsigned>(p.x, _mapSize.width - 1),
            std::min<unsigned>(p.y, _mapSize.height - 1)
        };
    }

    typename SelectionT::value_type toTarget(unsigned x, unsigned y);

    void boxSelection(SelectionT* sel, upoint p1, upoint p2, const bool selected)
    {
        p1 = clampPoint(p1);
        p2 = clampPoint(p2);

        const unsigned minX = std::min(p1.x, p2.x);
        const unsigned maxX = std::max(p1.x, p2.x);
        const unsigned minY = std::min(p1.y, p2.y);
        const unsigned maxY = std::max(p1.y, p2.y);

        for (unsigned y = minY; y <= maxY; y++) {
            for (unsigned x = minX; x <= maxX; x++) {
                const auto tile = toTarget(x, y);
                if (selected) {
                    sel->insert(tile);
                }
                else {
                    sel->erase(tile);
                }
            }
        }
    }

public:
    void reset(const SelectionT& sel, const usize& size)
    {
        if (_mapSize != size) {
            _boxSelectionStart = upoint(0, 0);
            _previousTilePos = upoint(0, 0);
        }
        _mapSize = size;

        _draging = false;
        _previousTileSelected = true;
        _previousSelection = sel;
    }

    void reset()
    {
        _boxSelectionStart = upoint(0, 0);
        _previousTilePos = upoint(0, 0);
        _mapSize = usize(0, 0);

        _draging = false;
        _clickedPosValid = false;
        _previousTileSelected = true;
        _previousSelection.clear();
    }

    // Must be called after `invisibleButton()`
    bool processSelection(SelectionT* sel,
                          const AbstractMetaTileEditorGui::Geometry& geo, const usize& size)
    {
        if (_mapSize != size) {
            _mapSize = size;
            assert(_mapSize.width < INT_MAX);
            assert(_mapSize.height < INT_MAX);

            sel->clear();
            _previousSelection.clear();
            _draging = false;
            _clickedPosValid = false;
            _boxSelectionStart = upoint(0, 0);
            _previousTilePos = upoint(0, 0);
        }

        if (ImGui::IsMouseDown(0) == false && _draging) {
            _previousSelection = *sel;
            _draging = false;
            _clickedPosValid = false;
        }

        if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
            if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape))) {
                bool notEmpty = !sel->empty();

                sel->clear();
                _previousSelection.clear();
                _draging = false;
                _clickedPosValid = false;

                return notEmpty;
            }

            if (ImGui::IsMouseClicked(0)) {
                const auto& io = ImGui::GetIO();
                const point sp = geo.toTilePos(io.MousePos);

                _draging = false;

                _clickedPosValid = sp.x >= 0 && sp.x < int(_mapSize.width)
                                   && sp.y >= 0 && sp.y < int(_mapSize.height);

                if (!_clickedPosValid) {
                    return false;
                }

                const upoint p(sp.x, sp.y);
                _previousTilePos = p;

                if (io.KeyShift) {
                    const bool s = io.KeyCtrl ? _previousTileSelected : true;

                    *sel = _previousSelection;
                    boxSelection(sel, _boxSelectionStart, p, s);
                }
                else {
                    auto tile = toTarget(p.x, p.y);

                    if (io.KeyCtrl) {
                        // Invert clicked tile
                        const auto it = sel->find(tile);
                        if (it == sel->end()) {
                            sel->insert(tile);
                            _previousTileSelected = true;
                        }
                        else {
                            sel->erase(it);
                            _previousTileSelected = false;
                        }
                    }
                    else {
                        // replace clicked tile
                        sel->clear();
                        sel->insert(tile);
                        _previousTileSelected = true;
                    }

                    _previousSelection = *sel;
                    _boxSelectionStart = p;
                }

                return true;
            }
            if (_clickedPosValid && ImGui::IsMouseDragging(0)) {
                const auto& io = ImGui::GetIO();
                const upoint p = clampPoint(geo.toTilePos(io.MousePos));

                if (p != _previousTilePos) {
                    _previousTilePos = p;

                    _draging = true;

                    if (io.KeyCtrl) {
                        *sel = _previousSelection;
                        boxSelection(sel, _boxSelectionStart, p, _previousTileSelected);
                    }
                    else {
                        sel->clear();
                        boxSelection(sel, _boxSelectionStart, p, true);
                    }

                    return true;
                }
            }
        }
        else {
            _draging = false;
        }

        return false;
    }
};

template <>
upoint TileSelector<upoint_vectorset>::toTarget(unsigned x, unsigned y)
{
    return { x, y };
}

template <>
uint8_t TileSelector<vectorset<uint8_t>>::toTarget(unsigned x, unsigned y)
{
    return x + y * TILESET_WIDTH;
}

static TileSelector<vectorset<uint8_t>> tilesetSelector;
static TileSelector<upoint_vectorset> editableTilesSelector;
static TileSelector<upoint_vectorset> scratchpadTilesSelector;

AbstractMetaTileEditorData::AbstractMetaTileEditorData(ItemIndex itemIndex)
    : AbstractExternalFileEditorData(itemIndex)
{
}

AbstractMetaTileEditorGui::AbstractMetaTileEditorGui()
    : AbstractEditorGui()
    , _data(nullptr)
    , _tilesetShader()
    , _tilemap()
    , _currentEditMode(EditMode::SelectTiles)
    , _cursor()
    , _animationTimer()
    , _tilemapOutOfDate(true)
{
}

bool AbstractMetaTileEditorGui::setEditorData(AbstractEditorData* data)
{
    return (_data = dynamic_cast<AbstractMetaTileEditorData*>(data));
}

void AbstractMetaTileEditorGui::editorDataChanged()
{
    markTilemapOutOfDate();
}

void AbstractMetaTileEditorGui::markTilemapOutOfDate()
{
    _tilemapOutOfDate = true;
}

void AbstractMetaTileEditorGui::resetState()
{
    markTilemapOutOfDate();
    abandonPlacedTiles();
}

void AbstractMetaTileEditorGui::resetSelectorState()
{
    assert(_data);

    tilesetSelector.reset(_data->selectedTilesetTiles, usize(TILESET_WIDTH, TILESET_HEIGHT));
    editableTilesSelector.reset(_data->selectedTiles, _data->map().size());
    scratchpadTilesSelector.reset();
}

void AbstractMetaTileEditorGui::editorOpened()
{
    resetSelectorState();

    _animationTimer.reset();

    _tilesetShader.reset();
    _tilesetShader.setShowTiles(showTiles);
    _tilesetShader.setShowTileCollisions(showTileCollisions);
    _tilesetShader.setShowInteractiveTiles(showInteractiveTiles);

    _cursor.mapDirty = false;
    _cursor.currentlyEditing = false;
    // Do not change cursor

    markTilemapOutOfDate();
}

void AbstractMetaTileEditorGui::editorClosed()
{
    commitPlacedTiles();
}

static AbstractMetaTileEditorGui::Geometry blankGeometry()
{
    constexpr unsigned width = UnTech::Rooms::RoomInput::MIN_MAP_WIDTH;
    constexpr unsigned height = UnTech::Rooms::RoomInput::MIN_MAP_HEIGHT;

    const ImVec2 zoom(1.0f, 1.0f);
    const ImVec2 tileSize(METATILE_SIZE_PX, METATILE_SIZE_PX);
    const ImVec2 mapRenderSize(width * METATILE_SIZE_PX, height * METATILE_SIZE_PX);
    const ImVec2 offset = centreOffset(mapRenderSize);

    return { tileSize, mapRenderSize, offset, zoom };
}

AbstractMetaTileEditorGui::Geometry AbstractMetaTileEditorGui::mapGeometry(const usize s, const ImVec2 zoom)
{
    // Prevent an ImGui::InvisibleButton assert failure
    if (s.width == 0 || s.height == 0) {
        return blankGeometry();
    }
    const ImVec2 tileSize(METATILE_SIZE_PX * zoom.x, METATILE_SIZE_PX * zoom.y);
    const ImVec2 mapRenderSize(s.width * tileSize.x, s.height * tileSize.y);
    const ImVec2 offset = centreOffset(mapRenderSize);

    return { tileSize, mapRenderSize, offset, zoom };
}

AbstractMetaTileEditorGui::Geometry AbstractMetaTileEditorGui::mapGeometryAutoZoom(const usize size)
{
    // Prevent an ImGui::InvisibleButton assert failure
    if (size.width == 0 || size.height == 0) {
        return blankGeometry();
    }
    const ImVec2 winSize = ImGui::GetWindowSize();
    const float zoom = std::max(std::floor(winSize.x / (size.width * METATILE_SIZE_PX + 8)), 1.0f);
    return mapGeometry(size, ImVec2(zoom, zoom));
}

AbstractMetaTileEditorGui::Geometry AbstractMetaTileEditorGui::tilesetGeometry(const ImVec2 zoom)
{
    return mapGeometry(usize(TILESET_WIDTH, TILESET_HEIGHT), zoom);
}

AbstractMetaTileEditorGui::Geometry AbstractMetaTileEditorGui::tilesetGeometryAutoZoom()
{
    return mapGeometryAutoZoom(usize(TILESET_WIDTH, TILESET_HEIGHT));
}

void AbstractMetaTileEditorGui::invisibleButton(const char* label, const Geometry& geo)
{
    ImGui::SetCursorScreenPos(geo.offset);
    ImGui::InvisibleButton(label, geo.mapSize);
}

void AbstractMetaTileEditorGui::showLayerButtons()
{
    ImGui::ToggledButtonWithTooltip("G##showGrid", &showGrid, "Show Grid");
    ImGui::SameLine();

    if (ImGui::ToggledButtonWithTooltip("T##showTiles", &showTiles, "Show Tiles")) {
        _tilesetShader.setShowTiles(showTiles);
    }
    ImGui::SameLine();

    if (ImGui::ToggledButtonWithTooltip("C##showTC", &showTileCollisions, "Show Tile Collisions")) {
        _tilesetShader.setShowTileCollisions(showTileCollisions);
    }
    ImGui::SameLine();

    if (ImGui::ToggledButtonWithTooltip("I##showIT", &showInteractiveTiles, "Show Interactive Tiles")) {
        _tilesetShader.setShowInteractiveTiles(showInteractiveTiles);
    }
    ImGui::SameLine();
}

void AbstractMetaTileEditorGui::viewMenu()
{
    ImGui::MenuItem("Show Grid", nullptr, &showGrid);

    if (ImGui::MenuItem("Show Tiles", nullptr, &showTiles)) {
        _tilesetShader.setShowTiles(showTiles);
    }

    if (ImGui::MenuItem("Show Tile Collisions", nullptr, &showTileCollisions)) {
        _tilesetShader.setShowTileCollisions(showTileCollisions);
    }

    if (ImGui::MenuItem("Show Interactive Tiles", nullptr, &showInteractiveTiles)) {
        _tilesetShader.setShowTileCollisions(showInteractiveTiles);
    }
}

void AbstractMetaTileEditorGui::drawGrid(ImDrawList* drawList, const Geometry& geo)
{
    // ::TODO clip these to visible area::
    const float startX = geo.offset.x;
    const float startY = geo.offset.y;
    const float endX = geo.offset.x + geo.mapSize.x;
    const float endY = geo.offset.y + geo.mapSize.y;

    const float maxX = endX + 1.0f;
    const float maxY = endY + 1.0f;

    drawList->AddRect(ImVec2(startX, startY), ImVec2(maxX, maxY), Style::gridColor);

    for (float x = startX + geo.tileSize.x; x < endX; x += geo.tileSize.x) {
        drawList->AddLine(ImVec2(x, startY), ImVec2(x, maxY), Style::gridColor);
    }

    for (float y = startY + geo.tileSize.y; y < endY; y += geo.tileSize.y) {
        drawList->AddLine(ImVec2(startX, y), ImVec2(maxX, y), Style::gridColor);
    }
}

void AbstractMetaTileEditorGui::tilesetInteractiveTilesTooltip(const AbstractMetaTileEditorGui::Geometry& geo)
{
    if (ImGui::IsItemHovered()) {
        const auto pos = geo.toTilePos(ImGui::GetMousePos());

        if (pos.x >= 0 && unsigned(pos.x) < TILESET_WIDTH
            && pos.y >= 0 && unsigned(pos.y) < TILESET_HEIGHT) {

            const uint8_t tileId = pos.y * TILESET_WIDTH + pos.x;

            const auto& iTile = tileFunctionTables().at(tileId);
            if (iTile.isValid()) {
                ImGui::BeginTooltip();
                ImGui::TextUnformatted(iTile);
                ImGui::EndTooltip();
            }
        }
    }
}

void AbstractMetaTileEditorGui::interactiveTilesTooltip(const grid<uint8_t>& mapData, const Geometry& geo)
{
    if (ImGui::IsItemHovered()) {
        const auto pos = geo.toTilePos(ImGui::GetMousePos());

        if (pos.x >= 0 && unsigned(pos.x) < mapData.width()
            && pos.y >= 0 && unsigned(pos.y) < mapData.height()) {

            const uint8_t tileId = mapData.at(pos.x, pos.y);

            const auto& iTile = tileFunctionTables().at(tileId);
            if (iTile.isValid()) {
                ImGui::BeginTooltip();
                ImGui::TextUnformatted(iTile);
                ImGui::EndTooltip();
            }
        }
    }
}

void AbstractMetaTileEditorGui::drawTileset(const Geometry& geo)
{
    assert(_data);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    tilesetInteractiveTilesTooltip(geo);

    if (showTiles || showTileCollisions || showInteractiveTiles) {
        drawList->AddImage(_tilesetShader.texture().imguiTextureId(), geo.offset, geo.offset + geo.mapSize);
    }

    if (showGrid) {
        drawGrid(drawList, geo);
    }

    // Draw selection
    for (const uint8_t& tileId : _data->selectedTilesetTiles) {
        const unsigned x = tileId % TILESET_WIDTH;
        const unsigned y = tileId / TILESET_WIDTH;

        const ImVec2 pMin = geo.tilePosToVec2(x, y);
        const ImVec2 pMax = pMin + geo.tileSize + ImVec2(1.0f, 1.0f);

        drawList->AddRectFilled(pMin, pMax, Style::tileSelectionFillColor, 0.0f, ImDrawCornerFlags_None);
        drawList->AddRect(pMin, pMax, Style::tileSelectionOutlineColor, 0.0f, ImDrawCornerFlags_None);
    }

    const bool sc = tilesetSelector.processSelection(&_data->selectedTilesetTiles, geo,
                                                     usize(TILESET_WIDTH, TILESET_HEIGHT));
    if (sc) {
        createTileCursorFromTilesetSelection();
        if (_currentEditMode == EditMode::SelectTiles) {
            setEditMode(EditMode::PlaceTiles);
        }
        _data->selectedTilesetTilesChanged();
        selectionChanged();
    }
}

void AbstractMetaTileEditorGui::tilesetMinimapWindow(const char* label)
{
    assert(_data);

    ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(label, nullptr, ImGuiWindowFlags_HorizontalScrollbar)) {

        const auto geo = tilesetGeometryAutoZoom();
        invisibleButton("##Tileset", geo);
        drawTileset(geo);
    }
    ImGui::End();
}

void AbstractMetaTileEditorGui::minimapWindow(const char* label)
{
    assert(_data);

    ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(label, nullptr, ImGuiWindowFlags_HorizontalScrollbar)) {

        if (!_tilemap.empty()) {
            const auto& mapData = _data->map();

            const auto geo = mapGeometryAutoZoom(_tilemap.gridSize());

            invisibleButton("##Map", geo);
            drawTilemap(_tilemap, geo);
            drawSelection(_data->selectedTiles, geo);

            interactiveTilesTooltip(mapData, geo);

            const bool sc = editableTilesSelector.processSelection(&_data->selectedTiles, geo, _tilemap.gridSize());
            if (sc) {
                const auto& map = _data->map();

                createTileCursor(map, _data->selectedTiles);
                if (_currentEditMode == EditMode::SelectTiles) {
                    setEditMode(EditMode::PlaceTiles);
                }
                _data->selectedTilesChanged();
                selectionChanged();
            }
        }
    }
    ImGui::End();
}

bool AbstractMetaTileEditorGui::scratchpadMinimapWindow(const char* label, const Shaders::MtTilemap& tilemap,
                                                        const grid<uint8_t>& mapData, upoint_vectorset* sel)
{
    assert(_data);

    bool selChanged = false;

    ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(label, nullptr, ImGuiWindowFlags_HorizontalScrollbar)) {

        if (!tilemap.empty()) {
            const auto geo = mapGeometryAutoZoom(tilemap.gridSize());
            invisibleButton("##Map", geo);
            drawTilemap(tilemap, geo);
            drawSelection(*sel, geo);

            interactiveTilesTooltip(mapData, geo);

            const bool sc = scratchpadTilesSelector.processSelection(sel, geo, tilemap.gridSize());
            if (sc) {
                selChanged = true;

                createTileCursor(mapData, *sel);
                if (_currentEditMode == EditMode::SelectTiles) {
                    setEditMode(EditMode::PlaceTiles);
                }
            }
        }
    }
    ImGui::End();

    return selChanged;
}

void AbstractMetaTileEditorGui::drawTilemap(const Shaders::MtTilemap& tilemap,
                                            const Geometry& geo)
{
    assert(_data);

    if (tilemap.empty()) {
        return;
    }

    auto* drawList = ImGui::GetWindowDrawList();

    if (showTiles || showTileCollisions || showInteractiveTiles) {
        tilemap.addToDrawList(drawList, geo.offset, geo.mapSize, _tilesetShader);
    }

    if (showGrid) {
        drawGrid(drawList, geo);
    }
}

void AbstractMetaTileEditorGui::drawSelection(const upoint_vectorset& selection, const Geometry& geo)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Draw selection
    for (const upoint& p : selection) {
        const ImVec2 pMin = geo.tilePosToVec2(p);
        const ImVec2 pMax = pMin + geo.tileSize + ImVec2(1.0f, 1.0f);

        drawList->AddRectFilled(pMin, pMax, Style::tileSelectionFillColor, 0.0f, ImDrawCornerFlags_None);
        drawList->AddRect(pMin, pMax, Style::tileSelectionOutlineColor, 0.0f, ImDrawCornerFlags_None);
    }
}

void AbstractMetaTileEditorGui::drawAndEditMap(const Geometry& geo)
{
    assert(_data);

    if (_tilemap.empty()) {
        return;
    }

    const auto& mapData = _data->map();

    drawTilemap(_tilemap, geo);

    if (_currentEditMode != EditMode::PlaceTiles) {
        interactiveTilesTooltip(mapData, geo);
    }

    switch (_currentEditMode) {
    case EditMode::SelectObjects:
    case EditMode::None: {
        break;
    }

    case EditMode::SelectTiles: {
        drawSelection(_data->selectedTiles, geo);

        const bool sc = editableTilesSelector.processSelection(&_data->selectedTiles, geo, _tilemap.gridSize());
        if (sc) {
            const auto& map = _data->map();

            createTileCursor(map, _data->selectedTiles);
            // Do not change mode in the editor
            _data->selectedTilesChanged();
            selectionChanged();
        }
        break;
    }

    case EditMode::PlaceTiles: {
        // Common code for drawing modes
        if (ImGui::IsItemActive() || ImGui::IsItemHovered()) {
            if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape))) {
                setEditMode(EditMode::SelectTiles);
            }
            else {
                processEditMode(geo);
            }
        }
        else {
            // Commit tiles when map has lost focus
            commitPlacedTiles();
        }
    }
    }
}

void AbstractMetaTileEditorGui::processEditMode(const Geometry& geo)
{
    // Edit window has focus or editable map item is active

    if (_cursor.tiles.empty()) {
        return;
    }

    switch (_currentEditMode) {
    case EditMode::SelectObjects:
    case EditMode::SelectTiles:
    case EditMode::None: {
        break;
    }

    case EditMode::PlaceTiles: {
        const auto& io = ImGui::GetIO();
        const point p = geo.toTilePos(io.MousePos, _cursor.tiles.size());

        drawCursorTiles(_cursor.tiles, p, geo);

        if (ImGui::IsMouseClicked(0)) {
            _cursor.currentlyEditing = true;
            placeTiles(_cursor.tiles, p);
            _cursor.lastTilePos = p;
        }
        else if (_cursor.currentlyEditing && ImGui::IsMouseDown(0)) {
            if (p != _cursor.lastTilePos) {
                // ::TODO draw line between old position and new position::
                placeTiles(_cursor.tiles, p);
                _cursor.lastTilePos = p;
            }
        }
        else if (ImGui::IsMouseReleased(0)) {
            commitPlacedTiles();
        }
    }
    }
}

void AbstractMetaTileEditorGui::drawCursorTiles(const grid<uint16_t>& tiles, const point& cursorPos, const Geometry& geo)
{
    assert(_data);

    if (_tilemap.empty() || tiles.empty()) {
        return;
    }

    const auto& mapSize = _tilemap.gridSize();

    // ::TODO add cursor bounds::
    const rect bounds(-cursorPos.x, -cursorPos.y, mapSize.width, mapSize.height);

    const ImVec2 uvSize(1.0f / TILESET_WIDTH, 1.0f / TILESET_HEIGHT);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    const ImTextureID textureId = _tilesetShader.tilesTexture().imguiTextureId();

    const ImVec2 startingPos = geo.tilePosToVec2(cursorPos);

    // Draw tiles
    {
        drawList->PushTextureID(textureId);

        ImVec2 p = startingPos;

        auto it = tiles.cbegin();
        for (unsigned y = 0; y < tiles.height(); y++) {
            for (unsigned x = 0; x < tiles.width(); x++) {
                const auto tileId = *it++;

                if (tileId < N_METATILES) {
                    ImVec2 uv((tileId % TILESET_WIDTH) / float(TILESET_WIDTH),
                              (tileId / TILESET_WIDTH) / float(TILESET_HEIGHT));

                    const ImU32 tint = bounds.contains(point(x, y)) ? Style::tileCursorInBoundsTint : Style::tileCursorOutOfBoundsTint;

                    drawList->AddImage(textureId, p, p + geo.tileSize, uv, uv + uvSize, tint);
                }
                p.x += geo.tileSize.x;
            }
            p.x = startingPos.x;
            p.y += geo.tileSize.y;
        }
        assert(it == tiles.cend());

        drawList->PopTextureID();
    }

    // Draw tile outlines
    {
        ImVec2 p = startingPos;

        auto it = tiles.cbegin();
        for (unsigned y = 0; y < tiles.height(); y++) {
            for (unsigned x = 0; x < tiles.width(); x++) {
                const auto& tileId = *it++;

                if (tileId < N_METATILES) {
                    const ImU32 color = bounds.contains(point(x, y)) ? Style::tileCursorInBoundsOutline : Style::tileCursorOutOfBoundsOutline;
                    const ImVec2 pMax = p + geo.tileSize + ImVec2(1.0f, 1.0f);

                    drawList->AddRect(p, pMax, color, 0.0f, ImDrawCornerFlags_None);
                }
                p.x += geo.tileSize.x;
            }
            p.x = startingPos.x;
            p.y += geo.tileSize.y;
        }
        assert(it == tiles.cend());
    }
}

void AbstractMetaTileEditorGui::placeTiles(const grid<uint16_t>& tiles, const point cursorPos)
{
    assert(_data);

    auto& map = _data->map();
    if (map.empty() || tiles.empty()) {
        return;
    }

    assert(tiles.width() < INT_MAX && tiles.height() < INT_MAX);

    // ::TODO add cursor bounds::
    const rect mapBounds(0, 0, map.width(), map.height());

    // Get draw tile cursor boundary
    unsigned tiles_x1 = std::clamp<int>(cursorPos.x, mapBounds.left(), mapBounds.right());
    unsigned tiles_y1 = std::clamp<int>(cursorPos.y, mapBounds.top(), mapBounds.bottom());
    unsigned tiles_x2 = std::clamp<int>(cursorPos.x + int(tiles.width()), mapBounds.left(), mapBounds.right());
    unsigned tiles_y2 = std::clamp<int>(cursorPos.y + int(tiles.height()), mapBounds.top(), mapBounds.bottom());

    if (tiles_x1 >= tiles_x2 || tiles_y1 >= tiles_y2) {
        return;
    }

    bool changed = false;

    auto it = tiles.cbegin();
    for (unsigned y = 0; y < tiles.height(); y++) {
        int mapY = cursorPos.y + int(y);
        if (mapY >= mapBounds.top() && mapY < mapBounds.bottom()) {
            for (unsigned x = 0; x < tiles.width(); x++) {
                const auto tileId = *it++;

                if (tileId < N_METATILES) {
                    int mapX = cursorPos.x + int(x);
                    if (mapX >= mapBounds.left() && mapX < mapBounds.right()) {
                        if (map.at(mapX, mapY) != tileId) {
                            map.at(mapX, mapY) = tileId;
                            changed = true;
                        }
                    }
                }
            }
        }
        else {
            it += tiles.width();
        }
    }
    assert(it == tiles.cend());

    if (changed) {
        _tilemap.setMapData(map);

        if (_cursor.mapDirty) {
            // Expand _cursor.modifiedTiles
            auto expandAxis = [](unsigned& aabb_a, unsigned& aabb_w, const unsigned a1, const unsigned a2) {
                if (aabb_a > a1) {
                    aabb_w += aabb_a - a1;
                    aabb_a = a1;
                }
                assert(a2 > aabb_a);
                unsigned w = a2 - aabb_a;
                if (w > aabb_w) {
                    aabb_w = w;
                }
            };
            expandAxis(_cursor.modifiedTiles.x, _cursor.modifiedTiles.width, tiles_x1, tiles_x2);
            expandAxis(_cursor.modifiedTiles.y, _cursor.modifiedTiles.height, tiles_y1, tiles_y2);
        }
        else {
            _cursor.mapDirty = true;
            _cursor.modifiedTiles.x = tiles_x1;
            _cursor.modifiedTiles.y = tiles_y1;
            _cursor.modifiedTiles.width = tiles_x2 - tiles_x1;
            _cursor.modifiedTiles.height = tiles_y2 - tiles_y1;
        }
    }
}

void AbstractMetaTileEditorGui::commitPlacedTiles()
{
    assert(_data);

    if (_cursor.mapDirty) {
        _data->mapTilesPlaced(_cursor.modifiedTiles);
        _cursor.mapDirty = false;
    }
    _cursor.currentlyEditing = false;
}

void AbstractMetaTileEditorGui::abandonPlacedTiles()
{
    _cursor.mapDirty = false;
    _cursor.currentlyEditing = false;
}

void AbstractMetaTileEditorGui::setEditMode(EditMode mode)
{
    if (_currentEditMode != mode) {
        _currentEditMode = mode;

        commitPlacedTiles();
    }
}

void AbstractMetaTileEditorGui::animationButtons()
{
    if (ImGui::ToggledButton("P##Play", _animationTimer.isActive())) {
        _animationTimer.playPause();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted("Pause/Play Animations");
        ImGui::Text("Tileset Frame: %u", unsigned(_tilesetShader.tilesetFrame()));
        ImGui::Text("Palette Frame: %u", unsigned(_tilesetShader.paletteFrame()));
        ImGui::EndTooltip();
    }
    ImGui::SameLine();

    if (ImGui::ButtonWithTooltip("N##NextTileset", "Next Tileset Frame")) {
        _animationTimer.stop();
        _tilesetShader.nextTilesetFrame();
    }
    ImGui::SameLine();

    if (ImGui::ButtonWithTooltip("N##NextPalette", "Next Palette Frame")) {
        _animationTimer.stop();
        _tilesetShader.nextPaletteFrame();
    }
    ImGui::SameLine();
}

bool AbstractMetaTileEditorGui::selectObjectsButton()
{
    const bool clicked = ImGui::ToggledButton("Select Objects", _currentEditMode == EditMode::SelectObjects);
    if (clicked) {
        setEditMode(EditMode::SelectObjects);
    }
    return clicked;
}

void AbstractMetaTileEditorGui::editModeButtons()
{
    auto b = [&](const char* label, EditMode mode) {
        if (ImGui::ToggledButton(label, _currentEditMode == mode)) {
            setEditMode(mode);
        }
    };

    b("Select Tiles", EditMode::SelectTiles);
    ImGui::SameLine();
    b("Place Tiles", EditMode::PlaceTiles);
}

void AbstractMetaTileEditorGui::setTileCursor(grid<uint16_t>&& tileCursor)
{
    commitPlacedTiles();

    if (!tileCursor.empty()) {
        _cursor.tiles = std::move(tileCursor);
        setEditMode(EditMode::SelectTiles);
    }
}

template <typename SelectionT, typename GetCellF, typename GetTileIdF>
static grid<uint16_t> cursorFromSelection(const SelectionT& selection, const usize& mapSize,
                                          GetCellF getCell, GetTileIdF getTileId)
{
    if (selection.empty()) {
        return grid<uint16_t>();
    }

    unsigned minX = UINT_MAX;
    unsigned maxX = 0;
    unsigned minY = UINT_MAX;
    unsigned maxY = 0;

    for (const auto& s : selection) {
        const upoint p = getCell(s);
        if (p.x < minX) {
            minX = p.x;
        }
        if (p.x > maxX) {
            maxX = p.x;
        }
        if (p.y < minY) {
            minY = p.y;
        }
        if (p.y > maxY) {
            maxY = p.y;
        }
    }

    if (minX >= mapSize.width || maxX >= mapSize.width
        || minY >= mapSize.height || maxY >= mapSize.height) {
        return grid<uint16_t>();
    }

    grid<uint16_t> cursor(maxX - minX + 1,
                          maxY - minY + 1,
                          0xffff);

    for (const auto& s : selection) {
        const upoint p = getCell(s);
        const uint8_t tileId = getTileId(s);

        if (p.x < mapSize.width && p.y < mapSize.height) {
            cursor.set(p.x - minX, p.y - minY, tileId);
        }
    }

    return cursor;
}

void AbstractMetaTileEditorGui::createTileCursorFromTilesetSelection()
{
    assert(_data);

    setTileCursor(cursorFromSelection(
        _data->selectedTilesetTiles, usize(TILESET_WIDTH, TILESET_HEIGHT),
        [&](uint8_t tId) { return upoint(tId % TILESET_WIDTH, tId / TILESET_WIDTH); },
        [&](uint8_t tId) { return tId; }));
}

void AbstractMetaTileEditorGui::createTileCursor(const grid<uint8_t>& map, const upoint_vectorset& selection)
{
    setTileCursor(cursorFromSelection(
        selection, map.size(),
        [&](upoint p) { return p; },
        [&](upoint p) { return map.at(p); }));
}

void AbstractMetaTileEditorGui::updateMapAndProcessAnimations()
{
    if (_tilemapOutOfDate) {
        _tilemap.setMapData(_data->map());

        _tilemapOutOfDate = false;
    }

    const auto& mtData = _tilesetShader.tilesetData();
    const auto& palData = _tilesetShader.paletteData();

    if (mtData && palData) {
        _animationTimer.process(
            mtData->animatedTileset.animationDelay, [&] { _tilesetShader.nextTilesetFrame(); },
            palData->animationDelay, [&] { _tilesetShader.nextPaletteFrame(); });
    }
    else {
        _animationTimer.stop();
    }
}

}
