/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mttilesetresourceitem.h"
#include "mttilesetaccessors.h"
#include "mttilesetresourcelist.h"
#include "gui-qt/common/helpers.h"
#include "models/metatiles/metatiles-serializer.h"

using namespace UnTech::GuiQt::MetaTiles;

MtTilesetResourceItem::MtTilesetResourceItem(MtTilesetResourceList* parent, size_t index)
    : AbstractExternalResourceItem(parent, index)
    , _tileParameters(new MtTilesetTileParameters(this))
    , _scratchpadGrid(new MtTilesetScratchpadGrid(this))
    , _compiledData(nullptr)
{
    Q_ASSERT(index < mtTilesetList().size());

    setFilename(QString::fromStdString(tilesetInputItem().filename));
}

void MtTilesetResourceItem::setData(const MT::MetaTileTilesetInput& data)
{
    std::unique_ptr<DataT>& tileset = tilesetInputItem().value;
    Q_ASSERT(tileset);

    bool nameChange = tileset->name != data.name;
    bool animationDelayChanged = tileset->animationFrames.animationDelay != data.animationFrames.animationDelay;
    bool imagesChange = tileset->animationFrames.frameImageFilenames != data.animationFrames.frameImageFilenames;
    bool palettesChanged = tileset->palettes != data.palettes;

    *tileset = data;
    emit dataChanged();

    if (nameChange) {
        setName(QString::fromStdString(data.name));
    }
    if (animationDelayChanged) {
        emit this->animationDelayChanged();
    }
    if (imagesChange) {
        setExternalFiles(convertStringList(data.animationFrames.frameImageFilenames));
    }
    if (palettesChanged) {
        updateDependencies();
        emit this->palettesChanged();
    }
}

void MtTilesetResourceItem::updateDependencies()
{
    std::unique_ptr<DataT>& tileset = tilesetInputItem().value;
    Q_ASSERT(tileset);

    QVector<Dependency> deps;
    for (auto& p : tileset->palettes) {
        deps.append({ ResourceTypeIndex::PALETTE, QString::fromStdString(p) });
    }
    setDependencies(deps);
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
        updateDependencies();
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

    auto mtd = UnTech::MetaTiles::convertTileset(*tileset, *res, err);
    bool valid = mtd && mtd->validate(res->metaTileEngineSettings, err);

    if (valid) {
        _compiledData = std::move(mtd);
        return true;
    }
    else {
        _compiledData.release();
        return false;
    }
}
