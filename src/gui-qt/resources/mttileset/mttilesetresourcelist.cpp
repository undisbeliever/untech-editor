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

using namespace UnTech::GuiQt::Resources;

MtTilesetResourceList::MtTilesetResourceList(QObject* parent, ResourceTypeIndex typeIndex)
    : AbstractResourceList(parent, typeIndex)
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

const AbstractResourceList::AddResourceDialogSettings& MtTilesetResourceList::addResourceDialogSettings() const
{
    const static AbstractResourceList::AddResourceDialogSettings filter = {
        tr("Add MetaTile Tileset Resource"),
        QString::fromUtf8("UnTech MetaTile Tileset File (*.utmt)"),
        QString::fromUtf8("utmt"),
        true
    };

    return filter;
}

size_t MtTilesetResourceList::nItems() const
{
    return document()->resourcesFile()->metaTileTilesetFilenames.size();
}

MtTilesetResourceItem* MtTilesetResourceList::buildResourceItem(size_t index)
{
    return new MtTilesetResourceItem(this, index);
}

void MtTilesetResourceList::do_addResource(const std::string& filename)
{
    auto& fnList = document()->resourcesFile()->metaTileTilesetFilenames;
    fnList.emplace_back(filename);

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
    auto& fnList = document()->resourcesFile()->metaTileTilesetFilenames;

    Q_ASSERT(index < fnList.size());
    fnList.erase(fnList.begin() + index);
}
