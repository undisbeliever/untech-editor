/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourceitem.h"
#include "accessors.h"
#include "resourcelist.h"
#include "gui-qt/accessor/resourceitemundohelper.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/project.h"
#include "models/metatiles/metatiles-serializer.h"
#include "models/project/project.h"

using namespace UnTech::GuiQt::MetaTiles::MtTileset;

constexpr uint16_t ResourceItem::DEFAULT_SCRATCHPAD_TILE;

ResourceItem::ResourceItem(ResourceList* parent, size_t index)
    : AbstractExternalResourceItem(parent, index)
    , _metaTileTilesets(parent->metaTileTilesets())
    , _tileParameters(new MtTilesetTileParameters(this))
    , _scratchpadGrid(new MtTilesetScratchpadGrid(this))
    , _compiledData(nullptr)
{
    Q_ASSERT(index < mtTilesetList().size());

    setFilename(QString::fromStdString(tilesetInputItem().filename));

    connect(this, &ResourceItem::nameChanged,
            this, &ResourceItem::tilesetPropertiesChanged);

    connect(this, &ResourceItem::tilesetPropertiesChanged,
            this, &AbstractResourceItem::markUnchecked);
}

unsigned ResourceItem::nMetaTiles() const
{
    if (_compiledData) {
        return _compiledData->nMetaTiles();
    }
    else {
        return 0;
    }
}

void ResourceItem::updateExternalFiles()
{
    QStringList files;
    if (auto* d = data()) {
        files = convertStringList(d->animationFrames.frameImageFilenames);
    }

    setExternalFiles(files);
}

void ResourceItem::updateDependencies()
{
    std::unique_ptr<DataT>& tileset = tilesetInputItem().value;
    Q_ASSERT(tileset);

    QVector<Dependency> deps;
    for (auto& p : tileset->palettes) {
        deps.append({ ResourceTypeIndex::PALETTE, QString::fromStdString(p) });
    }
    setDependencies(deps);
}

void ResourceItem::saveResourceData(const std::string& filename) const
{
    auto* tileset = this->tilesetInput();

    if (tileset) {
        MT::saveMetaTileTilesetInput(*tileset, filename);
    }
}

bool ResourceItem::loadResourceData(ErrorList& err)
{
    auto& tilesetItem = tilesetInputItem();

    setFilename(QString::fromStdString(tilesetItem.filename));

    if (tilesetItem.filename.empty()) {
        err.addError("Missing filename");
        return false;
    }

    try {
        tilesetItem.loadFile();
        setName(QString::fromStdString(tilesetInput()->name));
        updateExternalFiles();
        updateDependencies();
        return true;
    }
    catch (const std::exception& ex) {
        tilesetItem.value = nullptr;

        err.addError(ex.what());
        return false;
    }
}

bool ResourceItem::compileResource(ErrorList& err)
{
    auto* tileset = this->tilesetInput();

    if (tileset == nullptr) {
        err.addError("Unable to load file");
        return false;
    }
    const auto& pro = project()->projectFile();
    Q_ASSERT(pro);

    auto mtd = UnTech::MetaTiles::convertTileset(*tileset, *pro, err);
    bool valid = mtd && mtd->validate(pro->metaTileEngineSettings, err);

    if (valid) {
        _compiledData = std::move(mtd);
        return true;
    }
    else {
        _compiledData.release();
        return false;
    }
}

bool ResourceItem::editTileset_setName(const UnTech::idstring& name)
{
    return UndoHelper(this).editName(name);
}

bool ResourceItem::editTileset_setPalettes(const std::vector<UnTech::idstring>& palettes)
{
    return UndoHelper(this).editField(
        palettes,
        tr("Edit Palette List"),
        [](MT::MetaTileTilesetInput& ti) -> std::vector<idstring>& { return ti.palettes; },
        [](ResourceItem& item) { emit item.palettesChanged();
                                          emit item.tilesetPropertiesChanged();
                                          item.updateDependencies(); });
}

bool ResourceItem::editTileset_setFrameImageFilenames(const std::vector<std::string>& images)
{
    return UndoHelper(this).editField(
        images,
        tr("Edit Frame Image List"),
        [](MT::MetaTileTilesetInput& ti) -> std::vector<std::string>& { return ti.animationFrames.frameImageFilenames; },
        [](ResourceItem& item) { emit item.tilesetPropertiesChanged();
                                          item.updateExternalFiles(); });
}

bool ResourceItem::editTileset_setAnimationDelay(unsigned delay)
{
    return UndoHelper(this).editField(
        delay,
        tr("Edit Animation Delay"),
        [](MT::MetaTileTilesetInput& ti) -> unsigned& { return ti.animationFrames.animationDelay; },
        [](ResourceItem& item) { emit item.animationDelayChanged();
                                          emit item.tilesetPropertiesChanged(); });
}

bool ResourceItem::editTileset_setBitDepth(unsigned bitDepth)
{
    return UndoHelper(this).editField(
        bitDepth,
        tr("Edit Bit Depth"),
        [](MT::MetaTileTilesetInput& ti) -> unsigned& { return ti.animationFrames.bitDepth; },
        [](ResourceItem& item) { emit item.tilesetPropertiesChanged(); });
}

bool ResourceItem::editTileset_setAddTransparentTile(bool addTransparentTile)
{
    return UndoHelper(this).editField(
        addTransparentTile,
        tr("Edit Add Transparent Tile"),
        [](MT::MetaTileTilesetInput& ti) -> bool& { return ti.animationFrames.addTransparentTile; },
        [](ResourceItem& item) { emit item.tilesetPropertiesChanged(); });
}
