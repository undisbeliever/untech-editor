/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourcelist.h"
#include "resourceitem.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/project.h"
#include "models/common/imagecache.h"
#include "models/project/project.h"

#include <QFileInfo>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Resources::Palette;

ResourceList::ResourceList(Project* project)
    : AbstractResourceList(project, ResourceTypeIndex::PALETTE)
{
}

UnTech::NamedList<RES::PaletteInput>& ResourceList::palettes() const
{
    return project()->projectFile()->palettes;
}

const QString ResourceList::resourceTypeNameSingle() const
{
    return tr("Palette");
}

const QString ResourceList::resourceTypeNamePlural() const
{
    return tr("Palettes");
}

size_t ResourceList::nItems() const
{
    return project()->projectFile()->palettes.size();
}

ResourceItem* ResourceList::buildResourceItem(size_t index)
{
    return new ResourceItem(this, index);
}

const QList<AbstractResourceList::AddResourceSettings>& ResourceList::addResourceSettings() const
{
    const static QList<AbstractResourceList::AddResourceSettings> filters = {
        { tr("Add Palette"),
          QString::fromUtf8("PNG Image (*.png)"),
          QString::fromUtf8("png"),
          .canCreateFile = false }
    };

    return filters;
}

ResourceItem* ResourceList::findResource(const QString& name) const
{
    return qobject_cast<ResourceItem*>(AbstractResourceList::findResource(name));
}

void ResourceList::do_addResource(int settingIndex, const std::string& filename)
{
    Q_ASSERT(settingIndex == 0);

    auto& palettes = this->palettes();

    palettes.insert_back();
    RES::PaletteInput& pal = palettes.back();

    QFileInfo fi = QString::fromStdString(filename);
    QString name = fi.baseName();
    IdstringValidator().fixup(name);

    pal.name = name.toStdString();
    pal.paletteImageFilename = filename;

    const auto& paletteImage = ImageCache::loadPngImage(filename);
    pal.rowsPerFrame = qBound(1U, paletteImage->size().height, 16U);
}

void ResourceList::do_removeResource(unsigned index)
{
    auto& palettes = this->palettes();

    Q_ASSERT(index < palettes.size());
    palettes.remove(index);
}
