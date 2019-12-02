/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "managers.h"
#include "accessors.h"
#include "resourceitem.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/project.h"
#include "gui-qt/resources/palette/resourcelist.h"

using namespace UnTech::GuiQt::MetaTiles::MtTileset;

MtTilesetPropertyManager::MtTilesetPropertyManager(QObject* parent)
    : AbstractPropertyManager(parent)
    , _tileset(nullptr)
{
    using Type = UnTech::GuiQt::PropertyType;

    addProperty(tr("Name"), NAME, Type::IDSTRING);
    addProperty(tr("Scratchpad Size"), SCRATCHPAD_SIZE, Type::SIZE, QSize(0, 0), QSize(255, 255));
    addProperty(tr("Palettes"), PALETTES, Type::IDSTRING_LIST);
    addPropertyGroup(tr("Animation Frames:"));
    addProperty(tr("Frame Images"), FRAME_IMAGES, Type::FILENAME_LIST,
                QStringLiteral("PNG Image (*.png)"));
    addProperty(tr("Animation Delay"), ANIMATION_DELAY, Type::UNSIGNED, 0, 0x10000);
    addProperty(tr("Bit Depth"), BIT_DEPTH, Type::COMBO, QStringList{ "2 bpp", "4 bpp", "8 bpp" }, QVariantList{ 2, 4, 8 });
    addProperty(tr("Add Transparent Tile"), ADD_TRANSPARENT_TILE, Type::BOOLEAN);
}

void MtTilesetPropertyManager::setResourceItem(AbstractResourceItem* abstractItem)
{
    ResourceItem* item = qobject_cast<ResourceItem*>(abstractItem);

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

        connect(_tileset, &ResourceItem::dataChanged,
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
        param1 = _tileset->project()->palettes()->itemNames();
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

    case SCRATCHPAD_SIZE:
        return fromUsize(ti->scratchpad.size());

    case PALETTES:
        return convertStringList(ti->palettes);

    case FRAME_IMAGES:
        return fromPathVector(ti->animationFrames.frameImageFilenames);

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

    switch ((PropertyId)id) {
    case NAME:
        return _tileset->editTileset_setName(value.toString().toStdString());

    case SCRATCHPAD_SIZE:
        return _tileset->scratchpadGrid()->editGrid_resizeGrid(
            toUsize(value.toSize()));

    case PALETTES:
        return _tileset->editTileset_setPalettes(
            toIdstringVector(value.toStringList()));

    case FRAME_IMAGES:
        return _tileset->editTileset_setFrameImageFilenames(toPathVector(value.toStringList()));

    case ANIMATION_DELAY:
        return _tileset->editTileset_setAnimationDelay(value.toUInt());

    case BIT_DEPTH:
        return _tileset->editTileset_setBitDepth(value.toUInt());

    case ADD_TRANSPARENT_TILE:
        return _tileset->editTileset_setAddTransparentTile(value.toBool());
    }

    return false;
}
