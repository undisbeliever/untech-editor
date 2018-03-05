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

MtTilesetPropertiesManager::MtTilesetPropertiesManager(QObject* parent)
    : AbstractPropertyManager(parent)
    , _tileset(nullptr)
{
    addProperty(tr("Name"), NAME);
    addListProperty(tr("Frame Images"), FRAME_IMAGES);
    addListProperty(tr("Palettes"), PALETTES);
    addProperty(tr("Animation Delay"), ANIMATION_DELAY);
    addProperty(tr("Bit Depth"), BIT_DEPTH);
    addProperty(tr("Add Transparent Tile"), ADD_TRANSPARENT_TILE);
}

ResourceTypeIndex MtTilesetPropertiesManager::resourceTypeIndex() const
{
    return ResourceTypeIndex::MT_TILESET;
}

void MtTilesetPropertiesManager::setResourceItem(AbstractResourceItem* abstractItem)
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
                this, &MtTilesetPropertiesManager::dataChanged);
    }

    emit dataChanged();
}

QVariant MtTilesetPropertiesManager::data(int id) const
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

    case FRAME_IMAGES:
        return convertStringList(ti->animationFrames.frameImageFilenames);

    case PALETTES:
        return convertStringList(ti->palettes);

    case ANIMATION_DELAY:
        return ti->animationFrames.animationDelay;

    case BIT_DEPTH:
        return ti->animationFrames.bitDepth;

    case ADD_TRANSPARENT_TILE:
        return ti->animationFrames.addTransparentTile;
    }

    return QVariant();
}

bool MtTilesetPropertiesManager::setData(int id, const QVariant& value)
{
    Q_ASSERT(_tileset);
    Q_ASSERT(_tileset->tilesetInput());

    MT::MetaTileTilesetInput newData = *_tileset->tilesetInput();

    switch ((PropertyId)id) {
    case NAME:
        newData.name = value.toString().toStdString();
        break;

    case FRAME_IMAGES:
        newData.animationFrames.frameImageFilenames = toStringVector(value.toStringList());
        break;

    case PALETTES:
        newData.palettes = toIdstringVector(value.toStringList());
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
