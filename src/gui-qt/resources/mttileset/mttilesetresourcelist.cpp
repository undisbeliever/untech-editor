/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mttilesetresourcelist.h"
#include "mttilesetresourceitem.h"
#include "gui-qt/common/idstringvalidator.h"
#include "models/metatiles/metatiles-serializer.h"

#include <QFileInfo>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Resources;

MtTilesetResourceList::MtTilesetResourceList(ResourceProject* project)
    : AbstractResourceList(project, ResourceTypeIndex::MT_TILESET)
{
}

const QString MtTilesetResourceList::resourceTypeNameSingle() const
{
    return tr("MetaTile Tileset");
}

const QString MtTilesetResourceList::resourceTypeNamePlural() const
{
    return tr("MetaTile Tilesets");
}

size_t MtTilesetResourceList::nItems() const
{
    return project()->resourcesFile()->metaTileTilesets.size();
}

MtTilesetResourceItem* MtTilesetResourceList::buildResourceItem(size_t index)
{
    return new MtTilesetResourceItem(this, index);
}

const QList<AbstractResourceList::AddResourceSettings>& MtTilesetResourceList::addResourceSettings() const
{
    const static QList<AbstractResourceList::AddResourceSettings> settings = {
        { tr("Add MetaTile Tileset"),
          QString::fromUtf8("UnTech MetaTile Tileset File (*.utmt)"),
          QString::fromUtf8("utmt"),
          true },
    };

    return settings;
}

void MtTilesetResourceList::do_addResource(int settingIndex, const std::string& filename)
{
    Q_ASSERT(settingIndex == 0);

    auto& metaTileTilesets = project()->resourcesFile()->metaTileTilesets;
    metaTileTilesets.insert_back(filename);

    QFileInfo fi(QString::fromStdString(filename));
    if (!fi.exists()) {
        QString name = fi.baseName();
        IdstringValidator().fixup(name);

        MT::MetaTileTilesetInput tilesetInput;
        tilesetInput.name = name.toStdString();
        MT::saveMetaTileTilesetInput(tilesetInput, filename);
    }
}

void MtTilesetResourceList::do_removeResource(unsigned index)
{
    auto& metaTileTilesets = project()->resourcesFile()->metaTileTilesets;

    Q_ASSERT(index < metaTileTilesets.size());
    metaTileTilesets.remove(index);
}
