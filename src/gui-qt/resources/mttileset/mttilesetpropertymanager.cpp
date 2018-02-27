/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mttilesetpropertymanager.h"
#include "mttilesetresourceitem.h"
#include "gui-qt/common/helpers.h"

using namespace UnTech::GuiQt::Resources;

MtTilesetPropertiesManager::MtTilesetPropertiesManager(QObject* parent)
    : AbstractPropertyManager(parent)
    , _tileset(nullptr)
{
    addProperty(tr("Name"), NAME);
    addProperty(tr("Frame Images"), FRAME_IMAGES);
    addProperty(tr("Palettes"), PALETTES);
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

    if (_tileset != item) {
        _tileset = item;

        setEnabled(_tileset != nullptr);
        if (_tileset) {
            setEnabled(_tileset->tilesetInput() != nullptr);
        }

        emit dataChanged();
    }
}

QVariant MtTilesetPropertiesManager::data(int id) const
{
    if (_tileset == nullptr) {
        return QVariant();
    }

    const std::unique_ptr<MT::MetaTileTilesetInput>& ti = _tileset->tilesetInput();
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
