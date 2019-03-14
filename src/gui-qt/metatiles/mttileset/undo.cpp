/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mttilesetaccessors.h"
#include "mttilesetresourceitem.h"
#include "gui-qt/accessor/gridundohelper.h"
#include "gui-qt/accessor/resourceitemundohelper.h"

using namespace UnTech::GuiQt::MetaTiles;

bool MtTilesetResourceItem::editTileset_setName(const UnTech::idstring& name)
{
    return UndoHelper(this).editName(name);
}

bool MtTilesetResourceItem::editTileset_setPalettes(const std::vector<UnTech::idstring>& palettes)
{
    return UndoHelper(this).editField(
        palettes,
        tr("Edit Palette List"),
        [](MT::MetaTileTilesetInput& ti) -> std::vector<idstring>& { return ti.palettes; },
        [](MtTilesetResourceItem& item) { emit item.palettesChanged();
                                          emit item.tilesetPropertiesChanged();
                                          item.updateDependencies(); });
}

bool MtTilesetResourceItem::editTileset_setFrameImageFilenames(const std::vector<std::string>& images)
{
    return UndoHelper(this).editField(
        images,
        tr("Edit Frame Image List"),
        [](MT::MetaTileTilesetInput& ti) -> std::vector<std::string>& { return ti.animationFrames.frameImageFilenames; },
        [](MtTilesetResourceItem& item) { emit item.tilesetPropertiesChanged();
                                          item.updateExternalFiles(); });
}

bool MtTilesetResourceItem::editTileset_setAnimationDelay(unsigned delay)
{
    return UndoHelper(this).editField(
        delay,
        tr("Edit Animation Delay"),
        [](MT::MetaTileTilesetInput& ti) -> unsigned& { return ti.animationFrames.animationDelay; },
        [](MtTilesetResourceItem& item) { emit item.animationDelayChanged();
                                          emit item.tilesetPropertiesChanged(); });
}

bool MtTilesetResourceItem::editTileset_setBitDepth(unsigned bitDepth)
{
    return UndoHelper(this).editField(
        bitDepth,
        tr("Edit Bit Depth"),
        [](MT::MetaTileTilesetInput& ti) -> unsigned& { return ti.animationFrames.bitDepth; },
        [](MtTilesetResourceItem& item) { emit item.tilesetPropertiesChanged(); });
}

bool MtTilesetResourceItem::editTileset_setAddTransparentTile(bool addTransparentTile)
{
    return UndoHelper(this).editField(
        addTransparentTile,
        tr("Edit Add Transparent Tile"),
        [](MT::MetaTileTilesetInput& ti) -> bool& { return ti.animationFrames.addTransparentTile; },
        [](MtTilesetResourceItem& item) { emit item.tilesetPropertiesChanged(); });
}

bool MtTilesetScratchpadGrid::editGrid_resizeGrid(const usize& size)
{
    return UndoHelper(this).resizeGrid(
        size, MtTilesetResourceItem::DEFAULT_SCRATCHPAD_TILE,
        tr("Resize scratchpad"));
}

bool MtTilesetScratchpadGrid::editGrid_placeTiles(const point& location, const GridT& tiles)
{
    const auto* data = resourceItem()->compiledData();
    if (data == nullptr) {
        return false;
    }
    const unsigned nMetaTiles = data->nMetaTiles();

    return UndoHelper(this).editCellsWithCroppingAndCellTest(
        location, tiles,
        tr("Place Tiles"),
        [&](const uint16_t& t) { return t < nMetaTiles; });
}
