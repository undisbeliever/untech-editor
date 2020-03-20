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
#include "models/project/project-data.h"

using namespace UnTech::GuiQt::MetaTiles::MtTileset;

ResourceItem::ResourceItem(ResourceList* parent, size_t index)
    : AbstractExternalResourceItem(parent, index)
    , _metaTileTilesets(parent->metaTileTilesets())
    , _tileParameters(new MtTilesetTileParameters(this))
    , _scratchpadGrid(new MtTilesetScratchpadGrid(this))
{
    Q_ASSERT(index < mtTilesetList().size());

    setFilename(QString::fromStdString(tilesetInputItem().filename));

    connect(this, &ResourceItem::nameChanged,
            this, &ResourceItem::tilesetPropertiesChanged);

    connect(this, &ResourceItem::tilesetPropertiesChanged,
            this, &AbstractResourceItem::markUnchecked);
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
    deps.append({ ResourceTypeIndex::PALETTE, QString::fromStdString(tileset->animationFrames.conversionPalette) });
    for (auto& p : tileset->palettes) {
        deps.append({ ResourceTypeIndex::PALETTE, QString::fromStdString(p) });
    }
    setDependencies(deps);
}

void ResourceItem::saveResourceData(const std::filesystem::path& filename) const
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
        err.addErrorString("Missing filename");
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

        err.addErrorString(ex.what());
        return false;
    }
}

bool ResourceItem::compileResource(ErrorList& err)
{
    return project()->projectData().compileMetaTiles(this->index(), err);
}

const UnTech::optional<const UnTech::MetaTiles::MetaTileTilesetData&> ResourceItem::compiledData() const
{
    return project()->projectData().metaTileTilesets().at(index());
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

bool ResourceItem::editTileset_setFrameImageFilenames(const std::vector<std::filesystem::path>& images)
{
    return UndoHelper(this).editField(
        images,
        tr("Edit Frame Image List"),
        [](MT::MetaTileTilesetInput& ti) -> std::vector<std::filesystem::path>& { return ti.animationFrames.frameImageFilenames; },
        [](ResourceItem& item) { emit item.tilesetPropertiesChanged();
                                 item.updateExternalFiles(); });
}

bool ResourceItem::editTileset_setConversionPalette(const UnTech::idstring& paletteName)
{
    return UndoHelper(this).editField(
        paletteName,
        tr("Edit Conversion Palette"),
        [](MT::MetaTileTilesetInput& ti) -> idstring& { return ti.animationFrames.conversionPalette; },
        [](ResourceItem& item) { emit item.animationDelayChanged();
                                 emit item.tilesetPropertiesChanged();
                                 item.updateDependencies(); });
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
