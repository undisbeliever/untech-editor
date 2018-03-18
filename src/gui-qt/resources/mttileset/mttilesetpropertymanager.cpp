/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mttilesetpropertymanager.h"
#include "mttilesetresourceitem.h"
#include "../editresourceitemcommand.h"
#include "gui-qt/common/helpers.h"

using namespace UnTech::GuiQt::Resources;

MtTilesetPropertyManager::MtTilesetPropertyManager(QObject* parent)
    : AbstractPropertyManager(parent)
    , _tileset(nullptr)
{
    using Type = UnTech::GuiQt::PropertyType;

    addProperty(tr("Name"), NAME, Type::IDSTRING);
    addProperty(tr("Palettes"), PALETTES, Type::IDSTRING_LIST);
    addPropertyGroup(tr("Animation Frames:"));
    addProperty(tr("Frame Images"), FRAME_IMAGES, Type::FILENAME_LIST,
                QStringLiteral("PNG Image (*.png)"));
    addProperty(tr("Animation Delay"), ANIMATION_DELAY, Type::UNSIGNED, 0, 0x10000);
    addProperty(tr("Bit Depth"), BIT_DEPTH, Type::COMBO, QStringList{ "2 bpp", "4 bpp", "8 bpp" }, QVariantList{ 2, 4, 8 });
    addProperty(tr("Add Transparent Tile"), ADD_TRANSPARENT_TILE, Type::BOOLEAN);
}

ResourceTypeIndex MtTilesetPropertyManager::resourceTypeIndex() const
{
    return ResourceTypeIndex::MT_TILESET;
}

void MtTilesetPropertyManager::setResourceItem(AbstractResourceItem* abstractItem)
{
    MtTilesetResourceItem* item = qobject_cast<MtTilesetResourceItem*>(abstractItem);

    if (_tileset == item) {
        return;
    }
    if (_tileset) {
        _tileset->disconnect(this);
    }
    _tileset = item;

    setEnabled(_tileset != nullptr);

    if (_tileset) {
        setEnabled(_tileset->tilesetInput() != nullptr);

        connect(_tileset, &MtTilesetResourceItem::dataChanged,
                this, &MtTilesetPropertyManager::dataChanged);
    }

    emit dataChanged();
}

void MtTilesetPropertyManager::updateParameters(int id, QVariant& param1, QVariant&) const
{
    if (_tileset == nullptr) {
        return;
    }

    if (id == PALETTES) {
        param1 = _tileset->project()->resourceLists().at((int)ResourceTypeIndex::PALETTE)->itemNames();
    }
}

QVariant MtTilesetPropertyManager::data(int id) const
{
    if (_tileset == nullptr) {
        return QVariant();
    }

    const MT::MetaTileTilesetInput* ti = _tileset->tilesetInput();
    if (ti == nullptr) {
        return QVariant();
    }

    switch ((PropertyId)id) {
    case NAME:
        return QString::fromStdString(ti->name);

    case PALETTES:
        return convertStringList(ti->palettes);

    case FRAME_IMAGES:
        return convertStringList(ti->animationFrames.frameImageFilenames);

    case ANIMATION_DELAY:
        return ti->animationFrames.animationDelay;

    case BIT_DEPTH:
        return ti->animationFrames.bitDepth;

    case ADD_TRANSPARENT_TILE:
        return ti->animationFrames.addTransparentTile;
    }

    return QVariant();
}

bool MtTilesetPropertyManager::setData(int id, const QVariant& value)
{
    Q_ASSERT(_tileset);
    Q_ASSERT(_tileset->tilesetInput());

    MT::MetaTileTilesetInput newData = *_tileset->tilesetInput();

    switch ((PropertyId)id) {
    case NAME:
        newData.name = value.toString().toStdString();
        break;

    case PALETTES:
        newData.palettes = toIdstringVector(value.toStringList());
        break;

    case FRAME_IMAGES:
        newData.animationFrames.frameImageFilenames = toStringVector(value.toStringList());
        break;

    case ANIMATION_DELAY:
        newData.animationFrames.animationDelay = value.toUInt();
        break;

    case BIT_DEPTH:
        newData.animationFrames.bitDepth = value.toUInt();
        break;

    case ADD_TRANSPARENT_TILE:
        newData.animationFrames.addTransparentTile = value.toBool();
        break;
    }

    if (newData != *_tileset->tilesetInput()) {
        _tileset->undoStack()->push(
            new EditResourceItemCommand<MtTilesetResourceItem>(
                _tileset, *_tileset->tilesetInput(), newData,
                tr("Edit %1").arg(propertyTitle(id))));
        return true;
    }
    else {
        return false;
    }
}
