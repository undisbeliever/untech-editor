/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourceitem.h"
#include "resourcelist.h"
#include "gui-qt/accessor/resourceitemundohelper.h"
#include "gui-qt/project.h"
#include "models/project/project-data.h"

using namespace UnTech::GuiQt::Resources::BackgroundImage;

ResourceItem::ResourceItem(ResourceList* parent, size_t index)
    : AbstractInternalResourceItem(parent, index)
    , _backgroundImages(parent->backgroundImages())
{
    Q_ASSERT(index < _backgroundImages.size());

    setName(QString::fromStdString(backgroundImageInput().name));

    updateExternalFiles();
    updateDependencies();

    connect(this, &AbstractResourceItem::dataChanged,
            this, &AbstractResourceItem::markUnchecked);
}

void ResourceItem::updateExternalFiles()
{
    QStringList filenames;

    const RES::BackgroundImageInput& bi = backgroundImageInput();
    if (!bi.imageFilename.empty()) {
        filenames.append(QString::fromStdString(bi.imageFilename));
    }

    setExternalFiles(filenames);
}

void ResourceItem::updateDependencies()
{
    const RES::BackgroundImageInput& bi = backgroundImageInput();

    QVector<Dependency> deps = {
        { ResourceTypeIndex::PALETTE, QString::fromStdString(bi.conversionPlette) }
    };

    setDependencies(deps);
}

bool ResourceItem::compileResource(ErrorList& err)
{
    return project()->projectData().compileBackgroundImage(this->index(), err);
}

bool ResourceItem::edit_setName(const idstring& name)
{
    return UndoHelper(this).editName(name);
}

bool ResourceItem::edit_setBitDepth(const unsigned bitDepth)
{
    return UndoHelper(this).editField(
        bitDepth,
        tr("Edit Bit Depth"),
        [](RES::BackgroundImageInput& p) -> unsigned& { return p.bitDepth; });
}

bool ResourceItem::edit_setImageFilename(const std::filesystem::path& filename)
{
    return UndoHelper(this).editField(
        filename,
        tr("Edit Image Filename"),
        [](RES::BackgroundImageInput& p) -> std::filesystem::path& { return p.imageFilename; },
        [](ResourceItem& item) { item.updateExternalFiles(); });
}

bool ResourceItem::edit_setConversionPalette(const UnTech::idstring& paletteName)
{
    return UndoHelper(this).editField(
               paletteName,
               tr("Edit Conversion Palette"),
               [](RES::BackgroundImageInput& p) -> idstring& { return p.conversionPlette; }),
           [](ResourceItem& item) { item.updateDependencies(); };
}

bool ResourceItem::edit_setFirstPalette(unsigned firstPalette)
{
    return UndoHelper(this).editField(
        firstPalette,
        tr("Edit First Palette"),
        [](RES::BackgroundImageInput& p) -> unsigned& { return p.firstPalette; });
}

bool ResourceItem::edit_setNPalettes(unsigned nPalettes)
{
    return UndoHelper(this).editField(
        nPalettes,
        tr("Edit Number of Palette"),
        [](RES::BackgroundImageInput& p) -> unsigned& { return p.nPalettes; });
}

bool ResourceItem::edit_setDefaultOrder(bool defaultOrder)
{
    return UndoHelper(this).editField(
        defaultOrder,
        tr("Edit Default Order"),
        [](RES::BackgroundImageInput& p) -> bool& { return p.defaultOrder; });
}
