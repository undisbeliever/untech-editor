/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mttilesetresourceitem.h"
#include "accessors.h"
#include "mttilesetresourcelist.h"
#include "gui-qt/accessor/resourceitemundohelper.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/project.h"
#include "models/metatiles/metatiles-serializer.h"
#include "models/project/project.h"

using namespace UnTech::GuiQt::MetaTiles::MtTileset;

constexpr uint16_t MtTilesetResourceItem::DEFAULT_SCRATCHPAD_TILE;

MtTilesetResourceItem::MtTilesetResourceItem(MtTilesetResourceList* parent, size_t index)
    : AbstractExternalResourceItem(parent, index)
    , _metaTileTilesets(parent->metaTileTilesets())
    , _tileParameters(new MtTilesetTileParameters(this))
    , _scratchpadGrid(new MtTilesetScratchpadGrid(this))
    , _compiledData(nullptr)
{
    Q_ASSERT(index < mtTilesetList().size());

    setFilename(QString::fromStdString(tilesetInputItem().filename));

    connect(this, &MtTilesetResourceItem::nameChanged,
            this, &MtTilesetResourceItem::tilesetPropertiesChanged);

    connect(this, &MtTilesetResourceItem::tilesetPropertiesChanged,
            this, &AbstractResourceItem::markUnchecked);
}

unsigned MtTilesetResourceItem::nMetaTiles() const
{
    if (_compiledData) {
        return _compiledData->nMetaTiles();
    }
    else {
        return 0;
    }
}

void MtTilesetResourceItem::updateExternalFiles()
{
    QStringList files;
    if (auto* d = data()) {
        files = convertStringList(d->animationFrames.frameImageFilenames);
    }

    setExternalFiles(files);
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

bool MtTilesetResourceItem::loadResourceData(ErrorList& err)
{
    auto& tilesetItem = tilesetInputItem();

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

bool MtTilesetResourceItem::compileResource(ErrorList& err)
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

bool MtTilesetResourceItem::editTileset_setName(const UnTech::idstring& name)
{
    return UndoHelper(this).editName(name);
}

bool MtTilesetResourceItem::editTileset_setPalettes(const std::vector<UnTech::idstring>& palettes)
{
    return UndoHelper(this).editField(
        palettes,
        tr("Edit Palette List"),
        [](MT::MetaTileTilesetInput& ti) -> std::vector<idstring>& { return ti.palettes; },
        [](MtTilesetResourceItem& item) { emit item.palettesChanged();
                                          emit item.tilesetPropertiesChanged();
                                          item.updateDependencies(); });
}

bool MtTilesetResourceItem::editTileset_setFrameImageFilenames(const std::vector<std::string>& images)
{
    return UndoHelper(this).editField(
        images,
        tr("Edit Frame Image List"),
        [](MT::MetaTileTilesetInput& ti) -> std::vector<std::string>& { return ti.animationFrames.frameImageFilenames; },
        [](MtTilesetResourceItem& item) { emit item.tilesetPropertiesChanged();
                                          item.updateExternalFiles(); });
}

bool MtTilesetResourceItem::editTileset_setAnimationDelay(unsigned delay)
{
    return UndoHelper(this).editField(
        delay,
        tr("Edit Animation Delay"),
        [](MT::MetaTileTilesetInput& ti) -> unsigned& { return ti.animationFrames.animationDelay; },
        [](MtTilesetResourceItem& item) { emit item.animationDelayChanged();
                                          emit item.tilesetPropertiesChanged(); });
}

bool MtTilesetResourceItem::editTileset_setBitDepth(unsigned bitDepth)
{
    return UndoHelper(this).editField(
        bitDepth,
        tr("Edit Bit Depth"),
        [](MT::MetaTileTilesetInput& ti) -> unsigned& { return ti.animationFrames.bitDepth; },
        [](MtTilesetResourceItem& item) { emit item.tilesetPropertiesChanged(); });
}

bool MtTilesetResourceItem::editTileset_setAddTransparentTile(bool addTransparentTile)
{
    return UndoHelper(this).editField(
        addTransparentTile,
        tr("Edit Add Transparent Tile"),
        [](MT::MetaTileTilesetInput& ti) -> bool& { return ti.animationFrames.addTransparentTile; },
        [](MtTilesetResourceItem& item) { emit item.tilesetPropertiesChanged(); });
}
