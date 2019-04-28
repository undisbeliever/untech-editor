/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "gui-qt/accessor/gridundohelper.h"

using namespace UnTech::GuiQt::MetaTiles::MtTileset;

MtTilesetTileParameters::MtTilesetTileParameters(ResourceItem* tileset)
    : QObject(tileset)
    , _tileset(tileset)
    , _selectedIndexes()
{
    Q_ASSERT(tileset);

    connect(tileset, &ResourceItem::resourceLoaded,
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

MtTilesetScratchpadGrid::MtTilesetScratchpadGrid(ResourceItem* tileset)
    : AbstractGridAccessor(tileset)
{
    Q_ASSERT(tileset);

    connect(tileset, &ResourceItem::resourceLoaded,
            this, &MtTilesetScratchpadGrid::gridReset);

    connect(this, &MtTilesetScratchpadGrid::selectedCellsChanged,
            this, &MtTilesetScratchpadGrid::updateSelectedTileParameters);

    connect(this, &MtTilesetScratchpadGrid::gridChanged,
            this, &MtTilesetScratchpadGrid::updateSelectedTileParameters);
}

UnTech::usize MtTilesetScratchpadGrid::size() const
{
    if (auto* data = resourceItem()->data()) {
        return data->scratchpad.size();
    }
    return usize(0, 0);
}

void MtTilesetScratchpadGrid::updateSelectedTileParameters()
{
    const auto* data = resourceItem()->data();
    if (data == nullptr || selectedCells().empty()) {
        return;
    }

    const auto& scratchpad = data->scratchpad;

    vectorset<uint8_t> tiles;
    for (auto& p : selectedCells()) {
        if (p.x < scratchpad.width()
            && p.y < scratchpad.height()) {

            tiles.insert(scratchpad.at(p));
        }
    }

    resourceItem()->tileParameters()->setSelectedIndexes(std::move(tiles));
}

bool MtTilesetScratchpadGrid::editGrid_resizeGrid(const usize& size)
{
    return UndoHelper(this).resizeGrid(
        size, ResourceItem::DEFAULT_SCRATCHPAD_TILE,
        tr("Resize scratchpad"));
}

bool MtTilesetScratchpadGrid::editGrid_placeTiles(const point& location, const grid<uint16_t>& tiles, const QString& text)
{
    const auto* data = resourceItem()->compiledData();
    if (data == nullptr) {
        return false;
    }

    return UndoHelper(this).editCellsWithCroppingAndCellTest(
        location, tiles, text,
        [&](const uint16_t& t) -> optional<uint8_t> { return t < MT::N_METATILES ? t : optional<uint8_t>{}; });
}
