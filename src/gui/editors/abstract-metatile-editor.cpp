/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstract-metatile-editor.h"
#include "gui/common/tilecollisionimage.h"
#include "gui/imgui-drawing.h"
#include "gui/imgui.h"
#include "gui/texture.h"
#include "models/common/imagecache.h"
#include "models/common/vectorset-upoint.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace UnTech::Gui {

static constexpr unsigned METATILE_SIZE_PX = MetaTiles::METATILE_SIZE_PX;
static constexpr unsigned N_METATILES = MetaTiles::N_METATILES;
static constexpr unsigned TILESET_WIDTH = MetaTiles::TILESET_WIDTH;
static constexpr unsigned TILESET_HEIGHT = MetaTiles::TILESET_HEIGHT;
static constexpr unsigned N_TILE_COLLISONS = MetaTiles::N_TILE_COLLISONS;

// ::TODO move into style::
static constexpr auto collisionTint = IM_COL32(192, 0, 192, 128);
static constexpr auto selectionFillColor = IM_COL32(0, 0, 128, 128);
static constexpr auto selectionOutlineColor = IM_COL32(0, 0, 255, 255);
static constexpr auto cursorInBoundsTint = IM_COL32(128, 255, 128, 255);
static constexpr auto cursorInBoundsOutline = IM_COL32(0, 128, 0, 255);
static constexpr auto cursorOutOfBoundsTint = IM_COL32(255, 128, 128, 255);
static constexpr auto cursorOutOfBoundsOutline = IM_COL32(128, 0, 0, 255);

const usize AbstractMetaTileEditor::TILESET_TEXTURE_SIZE{
    TILESET_WIDTH * METATILE_SIZE_PX,
    TILESET_HEIGHT* METATILE_SIZE_PX,
};

struct AbstractMetaTileEditor::CursorState {
    // Values greater than UINT8_MAX are transparent
    grid<uint16_t> tiles;

    point lastTilePos;

    urect modifiedTiles;

    // Tiles drawn by the editor but not committed yet
    bool mapDirty = false;
    bool currentlyEditing = false;
};
AbstractMetaTileEditor::CursorState AbstractMetaTileEditor::_cursor;

AbstractMetaTileEditor::EditMode AbstractMetaTileEditor::_currentEditMode = AbstractMetaTileEditor::EditMode::SelectTiles;

point AbstractMetaTileEditor::Geometry::toTilePos(const ImVec2 globalPos) const
{
    const ImVec2 mousePos = ImVec2(globalPos.x - offset.x, globalPos.y - offset.y);
    return point(std::floor(mousePos.x / tileSize.x), std::floor(mousePos.y / tileSize.y));
}

point AbstractMetaTileEditor::Geometry::toTilePos(const ImVec2 globalPos, const usize cursorSize) const
{
    const point p = toTilePos(globalPos);
    return point(p.x - int(cursorSize.width / 2), p.y - int(cursorSize.height / 2));
}

ImVec2 AbstractMetaTileEditor::Geometry::tilePosToVec2(const unsigned x, const unsigned y) const
{
    return ImVec2(offset.x + tileSize.x * x,
                  offset.y + tileSize.y * y);
}

ImVec2 AbstractMetaTileEditor::Geometry::tilePosToVec2(const upoint pos) const
{
    return tilePosToVec2(pos.x, pos.y);
}

ImVec2 AbstractMetaTileEditor::Geometry::tilePosToVec2(const point pos) const
{
    return ImVec2(offset.x + tileSize.x * pos.x,
                  offset.y + tileSize.y * pos.y);
}

static void invisibleButton(const char* label, const AbstractMetaTileEditor::Geometry& geo)
{
    ImGui::SetCursorScreenPos(geo.offset);
    ImGui::InvisibleButton(label, geo.mapSize);
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

    upoint clampPoint(const point& p)
    {
        return upoint{
            std::clamp<unsigned>(p.x, 0, _mapSize.width - 1),
            std::clamp<unsigned>(p.y, 0, _mapSize.height - 1)
        };
    }

    upoint clampPoint(const upoint& p)
    {
        return upoint{
            std::clamp<unsigned>(p.x, 0, _mapSize.width - 1),
            std::clamp<unsigned>(p.y, 0, _mapSize.height - 1)
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

    // Must be called after `invisibleButton()`
    bool processSelection(SelectionT* sel,
                          const AbstractMetaTileEditor::Geometry& geo, const usize& size)
    {
        if (_mapSize != size) {
            _mapSize = size;
            assert(_mapSize.width < INT_MAX);
            assert(_mapSize.height < INT_MAX);

            sel->clear();
            _previousSelection.clear();
            _draging = false;
            _boxSelectionStart = upoint(0, 0);
            _previousTilePos = upoint(0, 0);
        }

        if (ImGui::IsMouseDown(0) == false && _draging) {
            _previousSelection = *sel;
            _draging = false;
        }

        if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
            if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape))) {
                bool notEmpty = !sel->empty();

                sel->clear();
                _previousSelection.clear();

                return notEmpty;
            }

            if (ImGui::IsMouseClicked(0)) {
                const auto& io = ImGui::GetIO();
                const point sp = geo.toTilePos(io.MousePos);

                if (sp.x < 0 || sp.x >= int(_mapSize.width)
                    || sp.y < 0 || sp.y >= int(_mapSize.height)) {

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
                _draging = false;

                return true;
            }
            if (ImGui::IsMouseDragging(0)) {
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

Texture& AbstractMetaTileEditor::tilesetTexture()
{
    static Texture texture(TILESET_TEXTURE_SIZE);
    return texture;
}

Texture& AbstractMetaTileEditor::tilesetCollisionsTexture()
{
    static Texture texture(TILESET_TEXTURE_SIZE);
    return texture;
}

// ::TODO animation::
// ::TODO draw tiles from ProjectData::

AbstractMetaTileEditor::AbstractMetaTileEditor(ItemIndex itemIndex)
    : AbstractEditor(itemIndex)
    , _tilesetIndex(0)
    , _paletteIndex(0)
{
    markTexturesOutOfDate();
}

void AbstractMetaTileEditor::markTexturesOutOfDate()
{
    _tilesetTextureOutOfDate = true;
    _collisionTextureOutOfDate = true;
}

void AbstractMetaTileEditor::markTilesetTextureOutOfDate()
{
    _tilesetTextureOutOfDate = true;
}

void AbstractMetaTileEditor::markCollisionTextureOutOfDate()
{
    _collisionTextureOutOfDate = true;
}

void AbstractMetaTileEditor::setTilesetIndex(unsigned index)
{
    if (_tilesetIndex != index) {
        _tilesetIndex = index;

        resetState();

        _cursor.tiles = grid<uint16_t>();
        setEditMode(EditMode::SelectTiles);

        // ::TODO reset scratchpad tiles Selector::

        // ::TODO reset animation::
    }
}

void AbstractMetaTileEditor::setPaletteIndex(unsigned index)
{
    if (_paletteIndex != index) {
        _paletteIndex = index;

        // ::TODO reset animation::

        _tilesetTextureOutOfDate = true;
    }
}

void AbstractMetaTileEditor::resetState()
{
    markTexturesOutOfDate();
    abandonPlacedTiles();
}

void AbstractMetaTileEditor::editorOpened()
{
    tilesetSelector.reset(_selectedTilesetTiles, usize(TILESET_WIDTH, TILESET_HEIGHT));
    editableTilesSelector.reset(_selectedTiles, map().size());

    _cursor.mapDirty = false;
    _cursor.currentlyEditing = false;
    // Do not change cursor

    markTexturesOutOfDate();
}

void AbstractMetaTileEditor::editorClosed()
{
    commitPlacedTiles();
}

void AbstractMetaTileEditor::updateSelection()
{
    const auto oldPalette = _paletteSel.selectedIndex();
    const auto oldTileset = _tilesetFrameSel.selectedIndex();

    _paletteSel.update();
    _tilesetFrameSel.update();

    if (_paletteSel.selectedIndex() != oldPalette
        || _tilesetFrameSel.selectedIndex() != oldTileset) {

        _tilesetTextureOutOfDate = true;
    }
}

AbstractMetaTileEditor::Geometry AbstractMetaTileEditor::mapGeometry(const usize s, const ImVec2 zoom)
{
    const ImVec2 tileSize(METATILE_SIZE_PX * zoom.x, METATILE_SIZE_PX * zoom.y);
    const ImVec2 mapRenderSize(s.width * tileSize.x, s.height * tileSize.y);
    const ImVec2 offset = centreOffset(mapRenderSize);

    return { tileSize, mapRenderSize, offset, zoom };
}

AbstractMetaTileEditor::Geometry AbstractMetaTileEditor::mapGeometryAutoZoom(const usize size)
{
    const ImVec2 winSize = ImGui::GetWindowSize();
    const float zoom = std::max(std::floor(winSize.x / (size.width * METATILE_SIZE_PX + 8)), 1.0f);
    return mapGeometry(size, ImVec2(zoom, zoom));
}

AbstractMetaTileEditor::Geometry AbstractMetaTileEditor::tilesetGeometry(const ImVec2 zoom)
{
    return mapGeometry(usize(TILESET_WIDTH, TILESET_HEIGHT), zoom);
}

AbstractMetaTileEditor::Geometry AbstractMetaTileEditor::tilesetGeometryAutoZoom()
{
    return mapGeometryAutoZoom(usize(TILESET_WIDTH, TILESET_HEIGHT));
}

void AbstractMetaTileEditor::drawGrid(ImDrawList* drawList, const Geometry& geo)
{
    const ImU32 gridColor = IM_COL32(128, 128, 128, 128);

    // ::TODO clip these to visible area::
    const float startX = geo.offset.x;
    const float startY = geo.offset.y;
    const float endX = geo.offset.x + geo.mapSize.x;
    const float endY = geo.offset.y + geo.mapSize.y;

    const float maxX = endX + 1.0f;
    const float maxY = endY + 1.0f;

    drawList->AddRect(ImVec2(startX, startY), ImVec2(maxX, maxY), gridColor);

    for (float x = startX + geo.tileSize.x; x < endX; x += geo.tileSize.x) {
        drawList->AddLine(ImVec2(x, startY), ImVec2(x, maxY), gridColor);
    }

    for (float y = startY + geo.tileSize.y; y < endY; y += geo.tileSize.y) {
        drawList->AddLine(ImVec2(startX, y), ImVec2(maxX, y), gridColor);
    }
}

void AbstractMetaTileEditor::drawTileset(const Geometry& geo)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    drawList->AddImage(tilesetTexture().imguiTextureId(), geo.offset, geo.offset + geo.mapSize);
    drawList->AddImage(tilesetCollisionsTexture().imguiTextureId(), geo.offset, geo.offset + geo.mapSize, ImVec2(0, 0), ImVec2(1, 1), collisionTint);

    // ::TODO draw tile symbols::

    // ::TODO make optional (based on style)::
    drawGrid(drawList, geo);

    // Draw selection
    for (const uint8_t& tileId : _selectedTilesetTiles) {
        const unsigned x = tileId % TILESET_WIDTH;
        const unsigned y = tileId / TILESET_WIDTH;

        const ImVec2 pMin = geo.tilePosToVec2(x, y);
        const ImVec2 pMax = pMin + geo.tileSize + ImVec2(1.0f, 1.0f);

        drawList->AddRectFilled(pMin, pMax, selectionFillColor, 0.0f, ImDrawCornerFlags_None);
        drawList->AddRect(pMin, pMax, selectionOutlineColor, 0.0f, ImDrawCornerFlags_None);
    }

    invisibleButton("##Tileset", geo);
    const bool sc = tilesetSelector.processSelection(&_selectedTilesetTiles, geo,
                                                     usize(TILESET_WIDTH, TILESET_HEIGHT));
    if (sc) {
        createTileCursorFromTilesetSelection();
        if (_currentEditMode == EditMode::SelectTiles) {
            setEditMode(EditMode::PlaceTiles);
        }
        selectedTilesetTilesChanged();
    }
}

void AbstractMetaTileEditor::tilesetMinimapWindow(const char* label)
{
    if (ImGui::Begin(label, nullptr, ImGuiWindowFlags_HorizontalScrollbar)) {
        ImGui::SetWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

        const auto geo = tilesetGeometryAutoZoom();
        drawTileset(geo);
    }
    ImGui::End();
}

void AbstractMetaTileEditor::minimapWindow(const char* label)
{
    if (ImGui::Begin(label, nullptr, ImGuiWindowFlags_HorizontalScrollbar)) {
        ImGui::SetWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

        const auto& map = this->map();
        if (!map.empty()) {
            const auto geo = mapGeometryAutoZoom(map.size());
            drawTiles(map, geo);
            drawSelection(_selectedTiles, geo);

            const bool sc = editableTilesSelector.processSelection(&_selectedTiles, geo, map.size());
            if (sc) {
                createTileCursor(map, _selectedTiles);
                if (_currentEditMode == EditMode::SelectTiles) {
                    setEditMode(EditMode::PlaceTiles);
                }
                selectedTilesChanged();
            }
        }
    }
    ImGui::End();
}

void AbstractMetaTileEditor::drawTiles(const grid<uint8_t>& map, const Geometry& geo)
{
    if (map.empty()) {
        return;
    }

    const ImVec2 uvSize(1.0f / TILESET_WIDTH, 1.0f / TILESET_HEIGHT);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    auto drawTiles = [&](const Texture& texture, const ImU32 tint) {
        const ImTextureID textureId = texture.imguiTextureId();

        drawList->PushTextureID(textureId);

        ImVec2 p = geo.offset;

        // ::TODO only draw tiles inside scroll area::
        auto it = map.begin();
        for (unsigned y = 0; y < map.height(); y++) {
            for (unsigned x = 0; x < map.width(); x++) {
                const uint8_t tileId = *it++;

                ImVec2 uv((tileId % TILESET_WIDTH) / float(TILESET_WIDTH),
                          (tileId / TILESET_WIDTH) / float(TILESET_HEIGHT));

                drawList->AddImage(textureId, p, p + geo.tileSize, uv, uv + uvSize, tint);

                p.x += geo.tileSize.x;
            }
            p.x = geo.offset.x;
            p.y += geo.tileSize.y;
        }
        assert(it == map.end());

        drawList->PopTextureID();
    };
    // ::TODO make optional (based on style)::
    drawTiles(tilesetTexture(), IM_COL32_WHITE);
    drawTiles(tilesetCollisionsTexture(), collisionTint);

    // ::TODO draw symbols::

    // ::TODO make optional (based on style)::
    drawGrid(drawList, geo);

    invisibleButton("##Map", geo);
}

void AbstractMetaTileEditor::drawSelection(const upoint_vectorset& selection, const Geometry& geo)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Draw selection
    for (const upoint& p : selection) {
        const ImVec2 pMin = geo.tilePosToVec2(p);
        const ImVec2 pMax = pMin + geo.tileSize + ImVec2(1.0f, 1.0f);

        drawList->AddRectFilled(pMin, pMax, selectionFillColor, 0.0f, ImDrawCornerFlags_None);
        drawList->AddRect(pMin, pMax, selectionOutlineColor, 0.0f, ImDrawCornerFlags_None);
    }
}

void AbstractMetaTileEditor::drawAndEditMap(const Geometry& geo)
{
    const auto& map = this->map();
    if (map.empty()) {
        return;
    }

    drawTiles(map, geo);

    switch (_currentEditMode) {
    case EditMode::None: {
        break;
    }

    case EditMode::SelectTiles: {
        drawSelection(_selectedTiles, geo);

        const bool sc = editableTilesSelector.processSelection(&_selectedTiles, geo, map.size());
        if (sc) {
            createTileCursor(map, _selectedTiles);
            // Do not change mode in the editor
            selectedTilesChanged();
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

void AbstractMetaTileEditor::processEditMode(const Geometry& geo)
{
    // Edit window has focus or editable map item is active

    if (_cursor.tiles.empty()) {
        return;
    }

    switch (_currentEditMode) {
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

void AbstractMetaTileEditor::drawCursorTiles(const grid<uint16_t>& tiles, const point& cursorPos, const Geometry& geo)
{
    const auto& map = this->map();
    if (map.empty() || tiles.empty()) {
        return;
    }

    // ::TODO add cursor bounds::
    const rect bounds(-cursorPos.x, -cursorPos.y, map.width(), map.height());

    const ImVec2 uvSize(1.0f / TILESET_WIDTH, 1.0f / TILESET_HEIGHT);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    const ImTextureID textureId = tilesetTexture().imguiTextureId();

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

                    const ImU32 tint = bounds.contains(point(x, y)) ? cursorInBoundsTint : cursorOutOfBoundsTint;

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
                    const ImU32 color = bounds.contains(point(x, y)) ? cursorInBoundsOutline : cursorOutOfBoundsOutline;
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

void AbstractMetaTileEditor::placeTiles(const grid<uint16_t>& tiles, const point cursorPos)
{
    auto& map = this->map();
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

void AbstractMetaTileEditor::commitPlacedTiles()
{
    if (_cursor.mapDirty) {
        mapTilesPlaced(_cursor.modifiedTiles);
        _cursor.mapDirty = false;
    }
    _cursor.currentlyEditing = false;
}

void AbstractMetaTileEditor::abandonPlacedTiles()
{
    _cursor.mapDirty = false;
    _cursor.currentlyEditing = false;
}

void AbstractMetaTileEditor::setEditMode(AbstractMetaTileEditor::EditMode mode)
{
    if (_currentEditMode != mode) {
        _currentEditMode = mode;

        commitPlacedTiles();
    }
}

void AbstractMetaTileEditor::editModeButtons()
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

void AbstractMetaTileEditor::setTileCursor(grid<uint16_t>&& tileCursor)
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

void AbstractMetaTileEditor::createTileCursorFromTilesetSelection()
{
    setTileCursor(cursorFromSelection(
        _selectedTilesetTiles, usize(TILESET_WIDTH, TILESET_HEIGHT),
        [&](uint8_t tId) { return upoint(tId % TILESET_WIDTH, tId / TILESET_WIDTH); },
        [&](uint8_t tId) { return tId; }));
}

void AbstractMetaTileEditor::createTileCursor(const grid<uint8_t>& map, const upoint_vectorset& selection)
{
    setTileCursor(cursorFromSelection(
        selection, map.size(),
        [&](upoint p) { return p; },
        [&](upoint p) { return map.at(p); }));
}

void AbstractMetaTileEditor::updateTextures(const Project::ProjectFile& projectFile)
{
    if (_tilesetTextureOutOfDate == false && _collisionTextureOutOfDate == false) {
        return;
    }

    const auto* tileset = projectFile.metaTileTilesets.at(_tilesetIndex);
    if (tileset == nullptr) {
        tilesetTexture().replaceWithMissingImageSymbol();
        tilesetCollisionsTexture().replaceWithMissingImageSymbol();
        return;
    }

    if (_paletteSel.selectedIndex() < tileset->palettes.size()) {
        const auto& paletteName = tileset->palettes.at(_paletteSel.selectedIndex());
        setPaletteIndex(projectFile.palettes.indexOf(paletteName));
    }
    else {
        setPaletteIndex(INT_MAX);
    }

    if (_tilesetTextureOutOfDate) {
        updateTilesetTexture(*tileset);
    }

    if (_collisionTextureOutOfDate) {
        updateCollisionsTexture(*tileset);
    }
}

void AbstractMetaTileEditor::updateTilesetTexture(const MetaTiles::MetaTileTilesetInput& tileset)
{
    if (false) {
        // ::TODO draw tileset from projectData::
    }
    else {
        const auto& filenames = tileset.animationFrames.frameImageFilenames;

        unsigned i = _tilesetFrameSel.selectedIndex();
        if (i >= filenames.size()) {
            i = 0;
            _tilesetFrameSel.setSelected(0);
        }

        std::shared_ptr<const Image> image;
        if (i < filenames.size()) {
            image = ImageCache::loadPngImage(filenames.at(i));
        }

        if (image && image->size() == TILESET_TEXTURE_SIZE) {
            tilesetTexture().replace(*image);
        }
        else {
            tilesetTexture().replaceWithMissingImageSymbol();
        }
    }

    _tilesetTextureOutOfDate = false;
}

void AbstractMetaTileEditor::updateCollisionsTexture(const MetaTiles::MetaTileTilesetInput& tileset)
{
    static Image img(TILESET_TEXTURE_SIZE);

    assert(tileCollisionImage.size().width == TILE_COLLISION_IMAGE_WIDTH);
    assert(tileCollisionImage.size().height == TILE_COLLISION_IMAGE_HEIGHT);
    static_assert(TILE_COLLISION_IMAGE_HEIGHT / METATILE_SIZE_PX >= N_TILE_COLLISONS);

    // ::TODO run through valgrind, msan and asan::
    // ::MAYDO find a safer way to do this::

    auto it = tileset.tileCollisions.begin();
    for (unsigned y = 0; y < TILESET_TEXTURE_SIZE.height; y += METATILE_SIZE_PX) {
        rgba* imgBits = img.scanline(y);

        for (unsigned x = 0; x < TILESET_TEXTURE_SIZE.width; x += METATILE_SIZE_PX) {
            const unsigned tileCollisionType = unsigned(*it++);
            if (tileCollisionType < N_TILE_COLLISONS) {
                const unsigned tcYoffset = tileCollisionType * METATILE_SIZE_PX;

                for (unsigned ty = 0; ty < METATILE_SIZE_PX; ty++) {
                    const rgba* tcBits = tileCollisionImage.scanline(tcYoffset + ty);
                    rgba* imgTcBits = imgBits + ty * TILESET_TEXTURE_SIZE.width;

                    std::copy(tcBits, tcBits + METATILE_SIZE_PX, imgTcBits);
                }
            }

            imgBits += METATILE_SIZE_PX;
            assert(imgBits + (METATILE_SIZE_PX - 1) * TILESET_TEXTURE_SIZE.width <= img.data() + img.dataSize());
        }
    }
    assert(it == tileset.tileCollisions.end());

    tilesetCollisionsTexture().replace(img);

    _collisionTextureOutOfDate = false;
}

}
