/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "paletteresourcelist.h"
#include "paletteresourceitem.h"

using namespace UnTech::GuiQt::Resources;

PaletteResourceList::PaletteResourceList(QObject* parent, ResourceTypeIndex typeIndex)
    : AbstractResourceList(parent, typeIndex)
{
}

const QString PaletteResourceList::resourceTypeName() const
{
    return tr("Palettes");
}

size_t PaletteResourceList::nItems() const
{
    return document()->resourcesFile()->palettes.size();
}

PaletteResourceItem* PaletteResourceList::buildResourceItem(size_t index)
{
    return new PaletteResourceItem(this, index);
}
