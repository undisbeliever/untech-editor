/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourcelist.h"
#include "resourceitem.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/project.h"
#include "models/metatiles/metatiles-serializer.h"
#include "models/project/project.h"

#include <QFileInfo>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaTiles::MtTileset;

ResourceList::ResourceList(Project* project)
    : AbstractResourceList(project, ResourceTypeIndex::MT_TILESET)
{
}

UnTech::ExternalFileList<UnTech::MetaTiles::MetaTileTilesetInput>& ResourceList::metaTileTilesets() const
{
    return project()->projectFile()->metaTileTilesets;
}

const QString ResourceList::resourceTypeNameSingle() const
{
    return tr("MetaTile Tileset");
}

const QString ResourceList::resourceTypeNamePlural() const
{
    return tr("MetaTile Tilesets");
}

size_t ResourceList::nItems() const
{
    return metaTileTilesets().size();
}

ResourceItem* ResourceList::buildResourceItem(size_t index)
{
    return new ResourceItem(this, index);
}

const QVector<AbstractResourceList::AddResourceSettings>& ResourceList::addResourceSettings() const
{
    const static QVector<AbstractResourceList::AddResourceSettings> settings = {
        { .title = tr("Add MetaTile Tileset"),
          .filter = QString::fromUtf8("UnTech MetaTile Tileset File (*.utmt)"),
          .extension = QString::fromUtf8("utmt"),
          .canCreateFile = true },
    };

    return settings;
}

void ResourceList::do_addResource(int settingIndex, const std::string& filename)
{
    Q_ASSERT(settingIndex == 0);

    auto& metaTileTilesets = this->metaTileTilesets();
    metaTileTilesets.insert_back(filename);

    QFileInfo fi(QString::fromStdString(filename));
    if (!fi.exists()) {
        QString name = fi.baseName();
        IdstringValidator().fixup(name);

        MT::MetaTileTilesetInput tilesetInput;
        tilesetInput.name = name.toStdString();
        tilesetInput.scratchpad = grid<uint8_t>(16, 16, ResourceItem::DEFAULT_SCRATCHPAD_TILE);
        MT::saveMetaTileTilesetInput(tilesetInput, filename);
    }
}

void ResourceList::do_removeResource(unsigned index)
{
    auto& metaTileTilesets = this->metaTileTilesets();

    Q_ASSERT(index < metaTileTilesets.size());
    metaTileTilesets.remove(index);
}
