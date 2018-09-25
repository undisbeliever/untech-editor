/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mttilesetaccessors.h"
#include "gui-qt/accessor/gridundohelper.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaTiles;

template class Accessor::GridUndoHelper<MtTilesetScratchpadGrid>;

MtTilesetTileParameters::MtTilesetTileParameters(MtTilesetResourceItem* tileset)
    : QObject(tileset)
    , _tileset(tileset)
    , _selectedIndexes()
{
    Q_ASSERT(tileset);

    connect(tileset, &MtTilesetResourceItem::resourceLoaded,
            this, &MtTilesetTileParameters::clearSelection);
}

void MtTilesetTileParameters::setSelectedIndexes(const vectorset<index_type>& selected)
{
    if (_selectedIndexes != selected) {
        _selectedIndexes = selected;
        emit selectedIndexesChanged();
    }
}

void MtTilesetTileParameters::setSelectedIndexes(vectorset<index_type>&& selected)
{
    if (_selectedIndexes != selected) {
        _selectedIndexes = std::move(selected);
        emit selectedIndexesChanged();
    }
}

void MtTilesetTileParameters::clearSelection()
{
    if (!_selectedIndexes.empty()) {
        _selectedIndexes.clear();
        emit selectedIndexesChanged();
    }
}

const UnTech::usize MtTilesetScratchpadGrid::max_size(255, 255);

MtTilesetScratchpadGrid::MtTilesetScratchpadGrid(MtTilesetResourceItem* tileset)
    : QObject(tileset)
    , _tileset(tileset)
    , _selectedCells()
{
    Q_ASSERT(tileset);

    connect(tileset, &MtTilesetResourceItem::resourceLoaded,
            this, &MtTilesetScratchpadGrid::gridResized);

    connect(tileset, &MtTilesetResourceItem::resourceLoaded,
            this, &MtTilesetScratchpadGrid::clearSelection);

    connect(this, &MtTilesetScratchpadGrid::gridChanged,
            this, &MtTilesetScratchpadGrid::updateSelectedTileParameters);
}

void MtTilesetScratchpadGrid::setSelectedCells(const upoint_vectorset& selected)
{
    if (_selectedCells != selected) {
        _selectedCells = selected;
        emit selectedCellsChanged();

        updateSelectedTileParameters();
    }
}

void MtTilesetScratchpadGrid::setSelectedCells(upoint_vectorset&& selected)
{
    if (_selectedCells != selected) {
        _selectedCells = std::move(selected);
        emit selectedCellsChanged();

        updateSelectedTileParameters();
    }
}

void MtTilesetScratchpadGrid::clearSelection()
{
    if (!_selectedCells.empty()) {
        _selectedCells.clear();
        emit selectedCellsChanged();
    }
}

void MtTilesetScratchpadGrid::updateSelectedTileParameters()
{
    const auto* data = _tileset->data();
    if (data == nullptr || _selectedCells.empty()) {
        return;
    }

    const auto& scratchpad = data->scratchpad;

    vectorset<uint16_t> tiles;
    for (auto& p : _selectedCells) {
        if (p.x < scratchpad.width()
            && p.y < scratchpad.height()) {

            tiles.insert(scratchpad.at(p));
        }
    }

    _tileset->tileParameters()->setSelectedIndexes(std::move(tiles));
}
