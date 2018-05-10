/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mttilesetresourceitem.h"
#include "mttilesetresourcelist.h"
#include "gui-qt/common/helpers.h"
#include "models/metatiles/metatiles-serializer.h"

using namespace UnTech::GuiQt::Resources;

MtTilesetResourceItem::MtTilesetResourceItem(MtTilesetResourceList* parent, size_t index)
    : AbstractExternalResourceItem(parent, index)
{
    Q_ASSERT(index < mtTilesetList().size());

    setFilename(QString::fromStdString(tilesetInputItem().filename));
}

void MtTilesetResourceItem::setData(const MT::MetaTileTilesetInput& data)
{
    std::unique_ptr<DataT>& tileset = tilesetInputItem().value;
    Q_ASSERT(tileset);

    bool nameChange = tileset->name != data.name;
    bool imagesChange = tileset->animationFrames.frameImageFilenames != data.animationFrames.frameImageFilenames;

    *tileset = data;
    emit dataChanged();

    if (nameChange) {
        setName(QString::fromStdString(data.name));
    }
    if (imagesChange) {
        setExternalFiles(convertStringList(data.animationFrames.frameImageFilenames));
    }
}

void MtTilesetResourceItem::saveResourceData(const std::string& filename) const
{
    auto* tileset = this->tilesetInput();

    if (tileset) {
        MT::saveMetaTileTilesetInput(*tileset, filename);
    }
}

bool MtTilesetResourceItem::loadResourceData(RES::ErrorList& err)
{
    auto& tilesetItem = tilesetInputItem();

    if (tilesetItem.filename.empty()) {
        err.addError("Missing filename");
        return false;
    }

    try {
        tilesetItem.loadFile();
        setName(QString::fromStdString(tilesetInput()->name));
        setExternalFiles(convertStringList(tilesetInput()->animationFrames.frameImageFilenames));
        return true;
    }
    catch (const std::exception& ex) {
        tilesetItem.value = nullptr;

        err.addError(ex.what());
        return false;
    }
}

bool MtTilesetResourceItem::compileResource(RES::ErrorList& err)
{
    auto* tileset = this->tilesetInput();

    if (tileset == nullptr) {
        err.addError("Unable to load file");
        return false;
    }
    const auto& res = project()->resourcesFile();
    Q_ASSERT(res);

    const auto mtd = MetaTiles::convertTileset(*tileset, *res, err);
    return mtd && mtd->validate(res->metaTileEngineSettings, err);
}
