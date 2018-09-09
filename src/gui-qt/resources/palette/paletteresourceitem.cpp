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
    , _compiledData(nullptr)
{
    Q_ASSERT(index < palettesData().size());

    auto* pal = paletteData();

    setName(QString::fromStdString(pal->name));

    if (pal->paletteImageFilename.empty() == false) {
        setExternalFiles({ QString::fromStdString(pal->paletteImageFilename) });
    }
}

void PaletteResourceItem::setData(const UnTech::Resources::PaletteInput& data)
{
    auto* pal = palettesData().at(index());
    Q_ASSERT(pal);

    bool nameChange = pal->name != data.name;
    bool imageChange = pal->paletteImageFilename != data.paletteImageFilename;

    *pal = data;
    emit dataChanged();

    if (nameChange) {
        setName(QString::fromStdString(data.name));
    }
    if (imageChange) {
        QStringList filenames;
        if (pal->paletteImageFilename.empty() == false) {
            filenames << QString::fromStdString(pal->paletteImageFilename);
        }
        setExternalFiles(filenames);
    }
}

bool PaletteResourceItem::compileResource(RES::ErrorList& err)
{
    const auto& res = project()->resourcesFile();
    Q_ASSERT(res);
    const RES::PaletteInput* pal = paletteData();
    Q_ASSERT(pal);

    auto palData = RES::convertPalette(*pal, err);
    bool valid = palData && palData->validate(err);

    if (valid) {
        _compiledData = std::move(palData);
        return true;
    }
    else {
        _compiledData.release();
        return false;
    }
}
