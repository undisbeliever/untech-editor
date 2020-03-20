/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "managers.h"
#include "resourceitem.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/project.h"
#include "gui-qt/resources/palette/resourcelist.h"

using namespace UnTech::GuiQt::Resources::BackgroundImage;

BackgroundImagePropertyManager::BackgroundImagePropertyManager(QObject* parent)
    : PropertyListManager(parent)
    , _resourceItem(nullptr)
{
    using Type = UnTech::GuiQt::PropertyType;

    addProperty(tr("Name"), NAME, Type::IDSTRING);
    addProperty(tr("Bit Depth"), BIT_DEPTH, Type::COMBO, QStringList{ "2 bpp", "4 bpp", "8 bpp" }, QVariantList{ 2, 4, 8 });
    addProperty(tr("Image"), IMAGE_FILENAME, Type::FILENAME, QStringLiteral("PNG Image (*.png)"));
    addProperty(tr("Conversion Palette"), CONVERSION_PALETTE, Type::IDSTRING);
    addProperty(tr("First Palette"), FIRST_PALETTE, Type::UNSIGNED, 0, 31);
    addProperty(tr("Number of Palettes"), N_PALETTES, Type::UNSIGNED, 1, 8);
    addProperty(tr("Default Order"), DEFAULT_ORDER, Type::UNSIGNED, 0, 1);

    addPropertyGroup(tr("Compiled Data"));
    addProperty(tr("Number of Tiles"), N_TILES, Type::NOT_EDITABLE);
}

void BackgroundImagePropertyManager::setResourceItem(ResourceItem* item)
{
    if (_resourceItem == item) {
        return;
    }

    if (_resourceItem) {
        _resourceItem->disconnect(this);
    }
    _resourceItem = item;

    setEnabled(_resourceItem != nullptr);

    if (_resourceItem) {
        connect(_resourceItem, &ResourceItem::dataChanged,
                this, &BackgroundImagePropertyManager::dataChanged);
        connect(_resourceItem, &ResourceItem::resourceComplied,
                this, &BackgroundImagePropertyManager::dataChanged);
    }

    emit dataChanged();
}

void BackgroundImagePropertyManager::updateParameters(int id, QVariant& param1, QVariant& param2) const
{
    Q_UNUSED(param2);

    if (_resourceItem == nullptr) {
        return;
    }

    if (id == CONVERSION_PALETTE) {
        param1 = _resourceItem->project()->palettes()->itemNames();
    }
}

QVariant BackgroundImagePropertyManager::data(int id) const
{
    if (_resourceItem == nullptr) {
        return QVariant();
    }

    const RES::BackgroundImageInput& bi = _resourceItem->backgroundImageInput();

    switch ((PropertyId)id) {
    case NAME:
        return QString::fromStdString(bi.name);

    case BIT_DEPTH:
        return bi.bitDepth;

    case IMAGE_FILENAME:
        return fromPath(bi.imageFilename);

    case CONVERSION_PALETTE:
        return QString::fromStdString(bi.conversionPlette);

    case FIRST_PALETTE:
        return bi.firstPalette;

    case N_PALETTES:
        return bi.nPalettes;

    case DEFAULT_ORDER:
        return unsigned(bi.defaultOrder);

    case N_TILES:
        if (auto data = _resourceItem->compiledData()) {
            return unsigned(data->tiles.size());
        }
        else {
            return QVariant();
        }
    }

    return QVariant();
}

bool BackgroundImagePropertyManager::setData(int id, const QVariant& value)
{
    Q_ASSERT(_resourceItem);

    switch ((PropertyId)id) {
    case NAME:
        return _resourceItem->edit_setName(value.toString().toStdString());

    case BIT_DEPTH:
        return _resourceItem->edit_setBitDepth(value.toUInt());

    case IMAGE_FILENAME:
        return _resourceItem->edit_setImageFilename(toPath(value.toString()));

    case CONVERSION_PALETTE:
        return _resourceItem->edit_setConversionPalette(value.toString().toStdString());

    case FIRST_PALETTE:
        return _resourceItem->edit_setFirstPalette(value.toUInt());

    case N_PALETTES:
        return _resourceItem->edit_setNPalettes(value.toUInt());

    case DEFAULT_ORDER:
        return _resourceItem->edit_setDefaultOrder(bool(value.toUInt()));

    case N_TILES:
        return false;
    }

    return false;
}
