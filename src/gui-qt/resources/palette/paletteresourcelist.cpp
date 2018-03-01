/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "paletteresourcelist.h"
#include "paletteresourceitem.h"
#include "gui-qt/common/idstringvalidator.h"
#include "models/common/imagecache.h"

#include <QFileInfo>

using namespace UnTech::GuiQt::Resources;

PaletteResourceList::PaletteResourceList(QObject* parent, ResourceTypeIndex typeIndex)
    : AbstractResourceList(parent, typeIndex)
{
}

const QString PaletteResourceList::resourceTypeNameSingle() const
{
    return tr("Palette");
}

const QString PaletteResourceList::resourceTypeNamePlural() const
{
    return tr("Palettes");
}

const AbstractResourceList::AddResourceDialogSettings& PaletteResourceList::addResourceDialogSettings() const
{
    const static AbstractResourceList::AddResourceDialogSettings filter = {
        tr("Select Palette Image"),
        QString::fromUtf8("PNG Image (*.png)"),
        QString::fromUtf8("png"),
        .canCreateFile = false
    };

    return filter;
}

size_t PaletteResourceList::nItems() const
{
    return document()->resourcesFile()->palettes.size();
}

PaletteResourceItem* PaletteResourceList::buildResourceItem(size_t index)
{
    return new PaletteResourceItem(this, index);
}

void PaletteResourceList::do_addResource(const std::string& filename)
{
    auto& palettes = document()->resourcesFile()->palettes;

    palettes.emplace_back(std::make_unique<RES::PaletteInput>());
    auto& pal = palettes.back();

    QFileInfo fi = QString::fromStdString(filename);
    QString name = fi.baseName();
    IdstringValidator().fixup(name);

    pal->name = name.toStdString();
    pal->paletteImageFilename = filename;

    const auto& paletteImage = ImageCache::loadPngImage(filename);
    pal->rowsPerFrame = qBound(1U, paletteImage->size().height, 16U);
}

void PaletteResourceList::do_removeResource(unsigned index)
{
    auto& palettes = document()->resourcesFile()->palettes;

    Q_ASSERT(index < palettes.size());
    palettes.erase(palettes.begin() + index);
}
