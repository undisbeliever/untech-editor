/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "paletteresourceitem.h"

using namespace UnTech::GuiQt::Resources;

PaletteResourceItem::PaletteResourceItem(AbstractResourceList* parent, size_t index)
    : AbstractResourceItem(parent, index)
{
    Q_ASSERT(index < palettesData().size());
}

const QString PaletteResourceItem::name() const
{
    const auto& pal = palettesData().at(index());
    return QString::fromStdString(pal->name);
}

const QString PaletteResourceItem::filename() const
{
    return QString();
}

bool PaletteResourceItem::loadResourceData(UnTech::Resources::ErrorList&)
{
    return true;
}

bool PaletteResourceItem::compileResource(RES::ErrorList& err)
{
    const auto& res = _document->resourcesFile();
    Q_ASSERT(res);
    const auto& pal = paletteData();

    const auto palData = RES::convertPalette(*pal, err);
    return palData && palData->validate(err);
}
