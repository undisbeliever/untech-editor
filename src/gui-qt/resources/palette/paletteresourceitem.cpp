/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "paletteresourceitem.h"
#include "paletteresourcelist.h"
#include "gui-qt/accessor/resourceitemundohelper.h"

using namespace UnTech::GuiQt::Resources::Palette;

PaletteResourceItem::PaletteResourceItem(PaletteResourceList* parent, size_t index)
    : AbstractInternalResourceItem(parent, index)
    , _palettes(parent->palettes())
    , _compiledData(nullptr)
{
    Q_ASSERT(index < _palettes.size());

    setName(QString::fromStdString(paletteInput().name));

    updateExternalFiles();

    connect(this, &AbstractResourceItem::dataChanged,
            this, &AbstractResourceItem::markUnchecked);
}

void PaletteResourceItem::updateExternalFiles()
{
    QStringList filenames;

    const RES::PaletteInput& pal = paletteInput();
    if (!pal.paletteImageFilename.empty()) {
        filenames.append(QString::fromStdString(pal.paletteImageFilename));
    }

    setExternalFiles(filenames);
}

bool PaletteResourceItem::compileResource(ErrorList& err)
{
    auto palData = RES::convertPalette(paletteInput(), err);
    bool valid = palData && palData->validate(err);

    if (valid) {
        _compiledData = std::move(palData);
        return true;
    }
    else {
        _compiledData.release();
        return false;
    }
}

bool PaletteResourceItem::editPalette_setName(const idstring& name)
{
    return UndoHelper(this).editName(name);
}

bool PaletteResourceItem::editPalette_setImageFilename(const std::string& filename)
{
    return UndoHelper(this).editField(
        filename,
        tr("Edit Image Filename"),
        [](RES::PaletteInput& p) -> std::string& { return p.paletteImageFilename; },
        [](PaletteResourceItem& pal) { pal.updateExternalFiles(); });
}

bool PaletteResourceItem::editPalette_setRowsPerFrame(unsigned rowsPerFrame)
{
    return UndoHelper(this).editField(
        rowsPerFrame,
        tr("Edit Rows Per Frame"),
        [](RES::PaletteInput& p) -> unsigned& { return p.rowsPerFrame; });
}

bool PaletteResourceItem::editPalette_setAnimationDelay(unsigned animationDelay)
{
    return UndoHelper(this).editField(
        animationDelay,
        tr("Edit Animation Delay"),
        [](RES::PaletteInput& p) -> unsigned& { return p.animationDelay; });
}

bool PaletteResourceItem::editPalette_setSkipFirstFrame(bool skipFirstFrame)
{
    return UndoHelper(this).editField(
        skipFirstFrame,
        skipFirstFrame ? tr("Set Skip First Frame") : tr("Clear Skip First Frame"),
        [](RES::PaletteInput& p) -> bool& { return p.skipFirstFrame; });
}
