/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mttilesetresourcelist.h"
#include "mttilesetresourceitem.h"

using namespace UnTech::GuiQt::Resources;

MtTilesetResourceList::MtTilesetResourceList(QObject* parent, ResourceTypeIndex typeIndex)
    : AbstractResourceList(parent, typeIndex)
{
}

const QString MtTilesetResourceList::resourceTypeName() const
{
    return tr("MetaTile Tilesets");
}

size_t MtTilesetResourceList::nItems() const
{
    return document()->resourcesFile()->metaTileTilesetFilenames.size();
}

MtTilesetResourceItem* MtTilesetResourceList::buildResourceItem(size_t index)
{
    return new MtTilesetResourceItem(this, index);
}
