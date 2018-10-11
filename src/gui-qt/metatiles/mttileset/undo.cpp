/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mttilesetaccessors.h"
#include "mttilesetresourceitem.h"
#include "gui-qt/accessor/gridundohelper.h"
#include "gui-qt/accessor/resourceitemundohelper.h"

using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::MetaTiles;

using TilesetUndoHelper = UnTech::GuiQt::Accessor::ResourceItemUndoHelper<MtTilesetResourceItem>;

bool MtTilesetResourceItem::editTileset_setName(const UnTech::idstring& name)
{
    return TilesetUndoHelper(this).editName(name);
}

bool MtTilesetResourceItem::editTileset_setPalettes(const std::vector<UnTech::idstring>& palettes)
{
    return TilesetUndoHelper(this).editField(
        palettes,
        tr("Edit Palette List"),
        [](MT::MetaTileTilesetInput& ti) -> std::vector<idstring>& { return ti.palettes; },
        [](MtTilesetResourceItem& item) { emit item.palettesChanged();
                                          emit item.tilesetPropertiesChanged();
                                          item.updateDependencies(); });
}

bool MtTilesetResourceItem::editTileset_setFrameImageFilenames(const std::vector<std::string>& images)
{
    return TilesetUndoHelper(this).editField(
        images,
        tr("Edit Frame Image List"),
        [](MT::MetaTileTilesetInput& ti) -> std::vector<std::string>& { return ti.animationFrames.frameImageFilenames; },
        [](MtTilesetResourceItem& item) { emit item.tilesetPropertiesChanged();
                                          item.updateExternalFiles(); });
}

bool MtTilesetResourceItem::editTileset_setAnimationDelay(unsigned delay)
{
    return TilesetUndoHelper(this).editField(
        delay,
        tr("Edit Animation Delay"),
        [](MT::MetaTileTilesetInput& ti) -> unsigned& { return ti.animationFrames.animationDelay; },
        [](MtTilesetResourceItem& item) { emit item.animationDelayChanged();
                                          emit item.tilesetPropertiesChanged(); });
}

bool MtTilesetResourceItem::editTileset_setBitDepth(unsigned bitDepth)
{
    return TilesetUndoHelper(this).editField(
        bitDepth,
        tr("Edit Bit Depth"),
        [](MT::MetaTileTilesetInput& ti) -> unsigned& { return ti.animationFrames.bitDepth; },
        [](MtTilesetResourceItem& item) { emit item.tilesetPropertiesChanged(); });
}

bool MtTilesetResourceItem::editTileset_setAddTransparentTile(bool addTransparentTile)
{
    return TilesetUndoHelper(this).editField(
        addTransparentTile,
        tr("Edit Add Transparent Tile"),
        [](MT::MetaTileTilesetInput& ti) -> bool& { return ti.animationFrames.addTransparentTile; },
        [](MtTilesetResourceItem& item) { emit item.tilesetPropertiesChanged(); });
}

using MtTilesetScratchpadGridUndoHelper = GridUndoHelper<MtTilesetScratchpadGrid>;

template class UnTech::GuiQt::Accessor::GridUndoHelper<MtTilesetScratchpadGrid>;

bool MtTilesetScratchpadGrid::editGrid_resizeGrid(const usize& size)
{
    return MtTilesetScratchpadGridUndoHelper(this).resizeSelectedGrid(
        size, MtTilesetResourceItem::DEFAULT_SCRATCHPAD_TILE,
        tr("Resize scratchpad"));
}

bool MtTilesetScratchpadGrid::editGrid_placeTiles(const point& location, const GridT& tiles)
{
    const auto* data = _tileset->compiledData();
    if (data == nullptr) {
        return false;
    }
    const unsigned nMetaTiles = data->nMetaTiles();

    return MtTilesetScratchpadGridUndoHelper(this).editCellsInSelectedGridWithCroppingAndCellTest(
        location, tiles,
        tr("Place Tiles"),
        [&](const uint16_t& t) { return t < nMetaTiles; });
}
