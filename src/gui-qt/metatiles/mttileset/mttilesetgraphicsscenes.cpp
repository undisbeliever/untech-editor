/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mttilesetgraphicsscenes.h"
#include "accessors.h"
#include "mttilesetrenderer.h"
#include "resourceitem.h"

using namespace UnTech;
using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaTiles;
using namespace UnTech::GuiQt::MetaTiles::MtTileset;

const grid<uint8_t> MtTilesetGraphicsScene::_grid = []() {
    static_assert(MT::TILESET_WIDTH * MT::TILESET_HEIGHT == 256, "Invalid tileset size");

    UnTech::grid<uint8_t> grid(MT::TILESET_WIDTH, MT::TILESET_HEIGHT);

    unsigned i = 0;
    for (uint8_t& cell : grid) {
        cell = i++;
    }

    return grid;
}();

MtTilesetGraphicsScene::MtTilesetGraphicsScene(Style* style, MtTilesetRenderer* renderer, QObject* parent)
    : MtGraphicsScene(style, renderer, parent)
    , _gridSelection()
{
}

const MtGraphicsScene::grid_t& MtTilesetGraphicsScene::grid() const
{
    return _grid;
}

const upoint_vectorset& MtTilesetGraphicsScene::gridSelection() const
{
    return _gridSelection;
}

void MtTilesetGraphicsScene::setGridSelection(upoint_vectorset&& selectedCells)
{
    auto* tilesetItem = this->tilesetItem();
    if (tilesetItem != nullptr && !_grid.empty()) {
        vectorset<uint8_t> selectedTiles;
        selectedTiles.reserve(selectedCells.size());

        for (auto& p : selectedCells) {
            unsigned tileId = p.y * _grid.width() + p.x;
            if (tileId < MT::N_METATILES) {
                selectedTiles.insert(tileId);
            }
        }
        tilesetItem->tileParameters()->setSelectedIndexes(std::move(selectedTiles));

        tilesetItem->scratchpadGrid()->clearSelection();
    }
    else {
        if (!_gridSelection.empty()) {
            _gridSelection.clear();
            emit gridSelectionChanged();
        }
    }
}

void MtTilesetGraphicsScene::tilesetItemChanged(ResourceItem* newTileset, ResourceItem* oldTileset)
{
    onSelectedTileParametersChanged();

    if (oldTileset) {
        oldTileset->tileParameters()->disconnect();
    }

    if (newTileset) {
        connect(newTileset->tileParameters(), &MtTilesetTileParameters::selectedIndexesChanged,
                this, &MtTilesetGraphicsScene::onSelectedTileParametersChanged);
    }
}

void MtTilesetGraphicsScene::onSelectedTileParametersChanged()
{
    _gridSelection.clear();

    if (auto* ti = tilesetItem()) {
        const auto* tileParameters = ti->tileParameters();
        const unsigned gridWidth = _grid.width();

        for (const auto& t : tileParameters->selectedIndexes()) {
            _gridSelection.insert(upoint(t % gridWidth, t / gridWidth));
        }
    }

    emit gridSelectionChanged();
}

MtScratchpadGraphicsScene::MtScratchpadGraphicsScene(Style* style, MtTilesetRenderer* renderer, QObject* parent)
    : MtGraphicsScene(style, renderer, parent)
{
}

const MtGraphicsScene::grid_t& MtScratchpadGraphicsScene::grid() const
{
    if (auto* ti = tilesetItem()) {
        if (auto* data = ti->data()) {
            return data->scratchpad;
        }
    }

    return BLANK_GRID;
}

const upoint_vectorset& MtScratchpadGraphicsScene::gridSelection() const
{
    if (auto* ti = tilesetItem()) {
        return ti->scratchpadGrid()->selectedCells();
    }

    return BLANK_GRID_SELECTION;
}

void MtScratchpadGraphicsScene::setGridSelection(upoint_vectorset&& selectedCells)
{
    if (auto* ti = tilesetItem()) {
        return ti->scratchpadGrid()->setSelectedCells(std::move(selectedCells));
    }
}

void MtScratchpadGraphicsScene::tilesetItemChanged(ResourceItem* newTileset, ResourceItem* oldTileset)
{
    if (oldTileset) {
        oldTileset->scratchpadGrid()->disconnect(this);
    }

    emit gridResized();
    emit gridSelectionChanged();

    if (newTileset) {
        connect(newTileset->scratchpadGrid(), &MtTilesetScratchpadGrid::gridChanged,
                this, &MtScratchpadGraphicsScene::gridChanged);

        connect(newTileset->scratchpadGrid(), &MtTilesetScratchpadGrid::gridResized,
                this, &MtScratchpadGraphicsScene::gridResized);
        connect(newTileset->scratchpadGrid(), &MtTilesetScratchpadGrid::gridResized,
                this, &MtEditableScratchpadGraphicsScene::gridResized);

        connect(newTileset->scratchpadGrid(), &MtTilesetScratchpadGrid::selectedCellsChanged,
                this, &MtScratchpadGraphicsScene::gridSelectionChanged);
    }
}

MtEditableScratchpadGraphicsScene::MtEditableScratchpadGraphicsScene(Style* style, MtTilesetRenderer* renderer, QObject* parent)
    : MtEditableGraphicsScene(style, renderer, parent)
{
}

const MtGraphicsScene::grid_t& MtEditableScratchpadGraphicsScene::grid() const
{
    if (auto* ti = tilesetItem()) {
        if (auto* data = ti->data()) {
            return data->scratchpad;
        }
    }

    return BLANK_GRID;
}

const upoint_vectorset& MtEditableScratchpadGraphicsScene::gridSelection() const
{
    if (auto* ti = tilesetItem()) {
        return ti->scratchpadGrid()->selectedCells();
    }

    return BLANK_GRID_SELECTION;
}

void MtEditableScratchpadGraphicsScene::setGridSelection(upoint_vectorset&& selectedCells)
{
    if (auto* ti = tilesetItem()) {
        return ti->scratchpadGrid()->setSelectedCells(std::move(selectedCells));
    }
}

void MtEditableScratchpadGraphicsScene::placeTiles(const selection_grid_t& tiles, point location, const QString& text, bool firstClick)
{
    tilesetItem()->scratchpadGrid()->editGrid_placeTiles(location, tiles, text, firstClick);
}

void MtEditableScratchpadGraphicsScene::tilesetItemChanged(ResourceItem* newTileset, ResourceItem* oldTileset)
{
    if (oldTileset) {
        oldTileset->scratchpadGrid()->disconnect(this);
    }

    emit gridResized();
    emit gridSelectionChanged();

    if (newTileset) {
        connect(newTileset->scratchpadGrid(), &MtTilesetScratchpadGrid::gridChanged,
                this, &MtEditableScratchpadGraphicsScene::gridChanged);

        connect(newTileset->scratchpadGrid(), &MtTilesetScratchpadGrid::gridReset,
                this, &MtEditableScratchpadGraphicsScene::gridResized);
        connect(newTileset->scratchpadGrid(), &MtTilesetScratchpadGrid::gridResized,
                this, &MtEditableScratchpadGraphicsScene::gridResized);

        connect(newTileset->scratchpadGrid(), &MtTilesetScratchpadGrid::selectedCellsChanged,
                this, &MtEditableScratchpadGraphicsScene::gridSelectionChanged);
    }
}
