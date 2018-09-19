/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mtgraphicsscenes.h"
#include "mtgridgraphicsitem.h"
#include "mttilesetaccessors.h"
#include "mttilesetrenderer.h"
#include "mttilesetresourceitem.h"

using namespace UnTech;
using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaTiles;

const MtGraphicsScene::grid_t MtGraphicsScene::BLANK_GRID;
const upoint_vectorset MtGraphicsScene::BLANK_GRID_SELECTION;

MtGraphicsScene::MtGraphicsScene(Style* style, MtTilesetRenderer* renderer, QObject* parent)
    : QGraphicsScene(parent)
    , _style(style)
    , _renderer(renderer)
    , _gridGraphicsItem(new MtGridGraphicsItem(this))
    , _tilesetItem(nullptr)
{
    Q_ASSERT(style);
    Q_ASSERT(renderer);

    this->addItem(_gridGraphicsItem);

    connect(_renderer, &MtTilesetRenderer::tilesetItemChanged,
            this, &MtGraphicsScene::onRendererTilesetItemChanged);
}

void MtGraphicsScene::onRendererTilesetItemChanged()
{
    MtTilesetResourceItem* oldItem = _tilesetItem;

    if (_tilesetItem) {
        _tilesetItem->disconnect(this);
    }
    _tilesetItem = _renderer->tilesetItem();

    tilesetItemChanged(_tilesetItem, oldItem);
}

MtTilesetGraphicsScene::MtTilesetGraphicsScene(Style* style, MtTilesetRenderer* renderer, QObject* parent)
    : MtGraphicsScene(style, renderer, parent)
    , _grid()
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
        vectorset<uint16_t> selectedTiles;
        selectedTiles.reserve(selectedCells.size());

        for (auto& p : selectedCells) {
            unsigned tileId = p.y * _grid.width() + p.x;
            if (tileId < renderer()->nMetaTiles()) {
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

void MtTilesetGraphicsScene::tilesetItemChanged(MtTilesetResourceItem* newTileset, MtTilesetResourceItem* oldTileset)
{
    onTilesetCompiled();
    onSelectedTileParametersChanged();

    if (oldTileset) {
        oldTileset->tileParameters()->disconnect();
    }

    if (newTileset) {
        connect(newTileset, &MtTilesetResourceItem::resourceComplied,
                this, &MtTilesetGraphicsScene::onTilesetCompiled);

        connect(newTileset->tileParameters(), &MtTilesetTileParameters::selectedIndexesChanged,
                this, &MtTilesetGraphicsScene::onSelectedTileParametersChanged);
    }
}

void MtTilesetGraphicsScene::onTilesetCompiled()
{
    usize gSize(0, 0);
    if (auto* ti = tilesetItem()) {
        if (auto* cd = ti->compiledData()) {
            gSize = cd->sourceTileSize();
        }
    }

    if (grid().size() != gSize) {
        _grid = UnTech::grid<uint16_t>(gSize);

        uint16_t i = 0;
        for (uint16_t& cell : _grid) {
            cell = i++;
        }

        emit gridResized();
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

void MtScratchpadGraphicsScene::tilesetItemChanged(MtTilesetResourceItem* newTileset, MtTilesetResourceItem* oldTileset)
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

        connect(newTileset->scratchpadGrid(), &MtTilesetScratchpadGrid::selectedCellsChanged,
                this, &MtScratchpadGraphicsScene::gridSelectionChanged);
    }
}
