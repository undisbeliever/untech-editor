/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "gui-qt/accessor/gridundohelper.h"
#include "gui-qt/accessor/resourceitemundohelper.h"

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

MtTilesetTileParameters::SelectedProperties MtTilesetTileParameters::selectedTileProperties() const
{
    SelectedProperties state;

    assert(_tileset);
    const MT::MetaTileTilesetInput* data = _tileset->data();

    if (data == nullptr || _selectedIndexes.empty()) {
        state.tileCollisionSame = false;
        state.functionTableSame = false;
    }
    else {
        state.tileCollision = data->tileCollisions.at(_selectedIndexes.front());
        state.tileCollisionSame = true;

        state.functionTable = data->tileFunctionTables.at(_selectedIndexes.front());
        state.functionTableSame = true;

        for (auto it = _selectedIndexes.begin() + 1; it != _selectedIndexes.end(); it++) {
            const auto& tc = data->tileCollisions.at(*it);
            if (tc != state.tileCollision) {
                state.tileCollisionSame = false;
            }

            const auto& ft = data->tileFunctionTables.at(*it);
            if (ft != state.functionTable) {
                state.functionTableSame = false;
            }
        }
    }

    return state;
}

std::array<Qt::CheckState, 4> MtTilesetTileParameters::selectedTilePriorities() const
{
    std::array<Qt::CheckState, 4> state{};

    assert(_tileset);
    const MT::MetaTileTilesetInput* data = _tileset->data();

    if (data && !_selectedIndexes.empty()) {
        const auto firstTile = _selectedIndexes.front();
        for (unsigned subTile = 0; subTile < state.size(); subTile++) {
            state.at(subTile) = data->tilePriorities.getTilePriority(firstTile, subTile) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
        }

        for (auto it = _selectedIndexes.begin() + 1; it != _selectedIndexes.end(); it++) {
            for (unsigned subTile = 0; subTile < state.size(); subTile++) {
                const auto c = data->tilePriorities.getTilePriority(*it, subTile) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
                if (state.at(subTile) != c) {
                    state.at(subTile) = Qt::CheckState::PartiallyChecked;
                }
            }
        }
    }

    return state;
}

void MtTilesetTileParameters::editSelectedTiles_setTileCollision(MT::TileCollisionType tc)
{
    assert(_tileset);
    const MT::MetaTileTilesetInput* data = _tileset->data();
    if (data == nullptr
        || _selectedIndexes.empty()) {
        return;
    }

    if (_selectedIndexes.size() == 1) {
        editTile_setTileCollision(_selectedIndexes.front(), tc);
    }
    else {
        // We copy the entire tileCollisions variable as it's only 256 bytes
        // and probably faster than creating the indexes, oldValues and newValues vectors.
        auto tileCollisions = data->tileCollisions;
        for (auto i : _selectedIndexes) {
            tileCollisions.at(i) = tc;
        }
        editTiles_setTileCollisions(tileCollisions);
    }
}

void MtTilesetTileParameters::editTile_setTileCollision(size_t index, const MT::TileCollisionType& tc)
{
    using UndoHelper = Accessor::ResourceItemUndoHelper<ResourceItem>;

    UndoHelper(_tileset).editField(
        tc,
        tr("Edit Tile Collision"),
        [=](MT::MetaTileTilesetInput& ts) -> MT::TileCollisionType& { return ts.tileCollisions.at(index); },
        [](ResourceItem& ri) { emit ri.tileParameters()->tileCollisionsChanged(); });
}

void MtTilesetTileParameters::editTiles_setTileCollisions(const std::array<MT::TileCollisionType, MT::N_METATILES>& tileCollisions)
{
    using UndoHelper = Accessor::ResourceItemUndoHelper<ResourceItem>;

    UndoHelper(_tileset).editField(
        tileCollisions,
        tr("Edit Tile Collisions"),
        [=](MT::MetaTileTilesetInput & ts) -> auto& { return ts.tileCollisions; },
        [](ResourceItem& ri) { emit ri.tileParameters()->tileCollisionsChanged(); });
}

void MtTilesetTileParameters::editSelectedTiles_setFunctionTable(const idstring& ft)
{
    assert(_tileset);
    const MT::MetaTileTilesetInput* data = _tileset->data();
    if (data == nullptr
        || _selectedIndexes.empty()) {
        return;
    }

    if (_selectedIndexes.size() == 1) {
        editTile_setFunctionTable(_selectedIndexes.front(), ft);
    }
    else {
        auto tileFunctionTables = data->tileFunctionTables;
        for (auto i : _selectedIndexes) {
            tileFunctionTables.at(i) = ft;
        }
        editTiles_setFunctionTables(tileFunctionTables);
    }
}

void MtTilesetTileParameters::editTile_setFunctionTable(size_t index, const idstring& ft)
{
    using UndoHelper = Accessor::ResourceItemUndoHelper<ResourceItem>;

    UndoHelper(_tileset).editField(
        ft,
        tr("Edit Tile Function Table"),
        [=](MT::MetaTileTilesetInput& ts) -> idstring& { return ts.tileFunctionTables.at(index); },
        [](ResourceItem& ri) { emit ri.tileParameters()->tileFunctionTablesChanged(); });
}

void MtTilesetTileParameters::editTiles_setFunctionTables(const std::array<idstring, MT::N_METATILES>& tileFunctionTables)
{
    using UndoHelper = Accessor::ResourceItemUndoHelper<ResourceItem>;

    UndoHelper(_tileset).editField(
        tileFunctionTables,
        tr("Edit Tile Function Tables"),
        [=](MT::MetaTileTilesetInput & ts) -> auto& { return ts.tileFunctionTables; },
        [](ResourceItem& ri) { emit ri.tileParameters()->tileFunctionTablesChanged(); });
}

void MtTilesetTileParameters::editSelectedTiles_setTilePriority(unsigned subTile, bool value)
{
    assert(_tileset);
    const MT::MetaTileTilesetInput* data = _tileset->data();
    if (data == nullptr
        || _selectedIndexes.empty()) {
        return;
    }

    if (_selectedIndexes.size() == 1) {
        editTiles_setTilePriority(_selectedIndexes.front(), subTile, value);
    }
    else {
        // We copy the entire tileCollisions variable as it's only 128 bytes
        // and probably faster than creating the indexes, oldValues and newValues vectors.
        auto tilePriorities = data->tilePriorities;
        for (auto i : _selectedIndexes) {
            tilePriorities.setTilePriority(i, subTile, value);
        }
        editTiles_setTilePriorities(tilePriorities);
    }
}

void MtTilesetTileParameters::editTiles_setTilePriority(unsigned metaTile, unsigned subTile, bool value)
{
    using UndoHelper = Accessor::ResourceItemUndoHelper<ResourceItem>;

    const MT::MetaTileTilesetInput* mtInput = _tileset->data();
    if (mtInput == nullptr) {
        return;
    }

    const auto p = mtInput->tilePriorities.indexDataPairToSetTilePriority(metaTile, subTile, value);
    const unsigned index = p.first;
    const uint8_t byte = p.second;

    UndoHelper(_tileset).editField(
        byte,
        tr("Edit Tile Priority"),
        [=](MT::MetaTileTilesetInput& ts) -> uint8_t& { return ts.tilePriorities.data.at(index); },
        [](ResourceItem& ri) { emit ri.tileParameters()->tilePrioritiesChanged(); });
}

void MtTilesetTileParameters::editTiles_setTilePriorities(const UnTech::MetaTiles::TilePriorities& tilePriorities)
{
    using UndoHelper = Accessor::ResourceItemUndoHelper<ResourceItem>;

    UndoHelper(_tileset).editField(
        tilePriorities,
        tr("Edit Tile Priotities"),
        [=](MT::MetaTileTilesetInput & ts) -> auto& { return ts.tilePriorities; },
        [](ResourceItem& ri) { emit ri.tileParameters()->tilePrioritiesChanged(); });
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

bool MtTilesetScratchpadGrid::editGrid_placeTiles(const point& location, const grid<uint16_t>& tiles, const QString& text, bool firstClick)
{
    return UndoHelper(this).editCellsMergeWithCroppingAndCellTest(
        location, tiles, text, firstClick,
        [&](const uint16_t& t) -> optional<uint8_t> { return t < MT::N_METATILES ? t : optional<uint8_t>{}; });
}
