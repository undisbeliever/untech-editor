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
    Q_ASSERT(index < project()->projectFile()->palettes.size());

    auto* pal = paletteInput();
    setName(QString::fromStdString(pal->name));

    updateExternalFiles();

    connect(this, &AbstractResourceItem::dataChanged,
            this, &AbstractResourceItem::markUnchecked);
}

void PaletteResourceItem::updateExternalFiles()
{
    QStringList filenames;

    if (const auto* pal = paletteInput()) {
        if (pal->paletteImageFilename.empty() == false) {
            filenames.append(QString::fromStdString(pal->paletteImageFilename));
        }
    }

    setExternalFiles(filenames);
}

bool PaletteResourceItem::compileResource(ErrorList& err)
{
    const auto& res = project()->projectFile();
    Q_ASSERT(res);
    const RES::PaletteInput* pal = paletteInput();
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
