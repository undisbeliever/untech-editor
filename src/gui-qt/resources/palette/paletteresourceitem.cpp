/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "paletteresourceitem.h"
#include "paletteresourcelist.h"

using namespace UnTech::GuiQt::Resources;

PaletteResourceItem::PaletteResourceItem(PaletteResourceList* parent, size_t index)
    : AbstractInternalResourceItem(parent, index)
{
    Q_ASSERT(index < palettesData().size());

    setName(QString::fromStdString(paletteData()->name));
}

void PaletteResourceItem::setData(const UnTech::Resources::PaletteInput& data)
{
    const auto& pal = palettesData().at(index());
    Q_ASSERT(pal);

    bool nameChange = pal->name != data.name;
    bool imageChange = pal->paletteImageFilename != data.paletteImageFilename;

    *pal = data;
    emit dataChanged();

    if (nameChange) {
        setName(QString::fromStdString(data.name));
    }
    if (imageChange) {
        emit imageFilenameChanged();
    }
}

bool PaletteResourceItem::compileResource(RES::ErrorList& err)
{
    const auto& res = project()->resourcesFile();
    Q_ASSERT(res);
    const RES::PaletteInput* pal = paletteData();
    Q_ASSERT(pal);

    const auto palData = RES::convertPalette(*pal, err);
    return palData && palData->validate(err);
}
