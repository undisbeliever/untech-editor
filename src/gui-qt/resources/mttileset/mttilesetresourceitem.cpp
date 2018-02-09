/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mttilesetresourceitem.h"

using namespace UnTech::GuiQt::Resources;

MtTilesetResourceItem::MtTilesetResourceItem(AbstractResourceList* parent, size_t index)
    : AbstractResourceItem(parent, index)
{
    Q_ASSERT(index < mtTilesetFilenames().size());
}

const QString MtTilesetResourceItem::name() const
{
    const auto& mf = mtTilesetFilenames().at(index());
    return QString::fromStdString(mf);
}

const QString MtTilesetResourceItem::filename() const
{
    const auto& mf = mtTilesetFilenames().at(index());
    return QString::fromStdString(mf);
}
