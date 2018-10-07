/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "paletteresourceitem.h"
#include "gui-qt/accessor/resourceitemundohelper.h"
#include "models/common/imagecache.h"

using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::Resources;

using PaletteUndoHelper = ResourceItemUndoHelper<PaletteResourceItem>;

bool PaletteResourceItem::editPalette_setName(const idstring& name)
{
    return PaletteUndoHelper(this).editName(name);
}

bool PaletteResourceItem::editPalette_setImageFilename(const std::string& filename)
{
    return PaletteUndoHelper(this).editField(
        filename,
        tr("Edit Image Filename"),
        [](RES::PaletteInput& p) -> std::string& { return p.paletteImageFilename; },
        [](PaletteResourceItem& pal) { pal.updateExternalFiles(); });
}

bool PaletteResourceItem::editPalette_setRowsPerFrame(unsigned rowsPerFrame)
{
    return PaletteUndoHelper(this).editField(
        rowsPerFrame,
        tr("Edit Rows Per Frame"),
        [](RES::PaletteInput& p) -> unsigned& { return p.rowsPerFrame; });
}

bool PaletteResourceItem::editPalette_setAnimationDelay(unsigned animationDelay)
{
    return PaletteUndoHelper(this).editField(
        animationDelay,
        tr("Edit Animation Delay"),
        [](RES::PaletteInput& p) -> unsigned& { return p.animationDelay; });
}

bool PaletteResourceItem::editPalette_setSkipFirstFrame(bool skipFirstFrame)
{
    return PaletteUndoHelper(this).editField(
        skipFirstFrame,
        skipFirstFrame ? tr("Set Skip First Frame") : tr("Clear Skip First Frame"),
        [](RES::PaletteInput& p) -> bool& { return p.skipFirstFrame; });
}
