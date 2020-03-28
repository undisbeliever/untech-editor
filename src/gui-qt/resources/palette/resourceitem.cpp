/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourceitem.h"
#include "resourcelist.h"
#include "gui-qt/accessor/resourceitemundohelper.h"
#include "gui-qt/project.h"
#include "models/project/project-data.h"

using namespace UnTech::GuiQt::Resources::Palette;

ResourceItem::ResourceItem(ResourceList* parent, size_t index)
    : AbstractInternalResourceItem(parent, index)
    , _palettes(parent->palettes())
{
    Q_ASSERT(index < _palettes.size());

    setName(QString::fromStdString(paletteInput().name));

    updateExternalFiles();

    connect(this, &AbstractResourceItem::dataChanged,
            this, &AbstractResourceItem::markUnchecked);
}

void ResourceItem::updateExternalFiles()
{
    QStringList filenames;

    const RES::PaletteInput& pal = paletteInput();
    if (!pal.paletteImageFilename.empty()) {
        filenames.append(QString::fromStdString(pal.paletteImageFilename));
    }

    setExternalFiles(filenames);
}

bool ResourceItem::compileResource(ErrorList& err)
{
    return project()->projectData().compilePalette(this->index(), err);
}

bool ResourceItem::editPalette_setName(const idstring& name)
{
    return UndoHelper(this).editName(name);
}

bool ResourceItem::editPalette_setImageFilename(const std::filesystem::path& filename)
{
    return UndoHelper(this).editField(
        filename,
        tr("Edit Image Filename"),
        [](RES::PaletteInput& p) -> std::filesystem::path& { return p.paletteImageFilename; },
        [](ResourceItem& pal) { pal.updateExternalFiles(); });
}

bool ResourceItem::editPalette_setRowsPerFrame(unsigned rowsPerFrame)
{
    return UndoHelper(this).editField(
        rowsPerFrame,
        tr("Edit Rows Per Frame"),
        [](RES::PaletteInput& p) -> unsigned& { return p.rowsPerFrame; });
}

bool ResourceItem::editPalette_setAnimationDelay(unsigned animationDelay)
{
    return UndoHelper(this).editField(
        animationDelay,
        tr("Edit Animation Delay"),
        [](RES::PaletteInput& p) -> unsigned& { return p.animationDelay; });
}

bool ResourceItem::editPalette_setSkipFirstFrame(bool skipFirstFrame)
{
    return UndoHelper(this).editField(
        skipFirstFrame,
        skipFirstFrame ? tr("Set Skip First Frame") : tr("Clear Skip First Frame"),
        [](RES::PaletteInput& p) -> bool& { return p.skipFirstFrame; });
}
