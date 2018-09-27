/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mttilesetpropertymanager.h"
#include "mttilesetaccessors.h"
#include "mttilesetresourceitem.h"
#include "gui-qt/accessor/gridundohelper.h"
#include "gui-qt/accessor/resourceitemundohelper.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/resources/palette/paletteresourcelist.h"

using namespace UnTech::GuiQt::MetaTiles;

using TilesetUndoHelper = UnTech::GuiQt::Accessor::ResourceItemUndoHelper<MtTilesetResourceItem>;

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
        param1 = _tileset->project()->paletteResourceList()->itemNames();
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
    Q_ASSERT(_tileset->data());

    TilesetUndoHelper undoHelper(_tileset);

    switch ((PropertyId)id) {
    case NAME:
        return undoHelper.editName(value.toString().toStdString());

    case SCRATCHPAD_SIZE:
        return MtTilesetScratchpadGridUndoHelper(_tileset->scratchpadGrid())
            .resizeSelectedGrid(toUsize(value.toSize()), 0,
                                tr("Resize scratchpad"));

    case PALETTES:
        return undoHelper.editField(toIdstringVector(value.toStringList()),
                                    tr("Edit Palette List"),
                                    [](MT::MetaTileTilesetInput& ti) -> std::vector<idstring>& { return ti.palettes; },
                                    [](MtTilesetResourceItem& item) { emit item.palettesChanged();
                                                                      item.updateDependencies(); });

    case FRAME_IMAGES:
        return undoHelper.editField(toStringVector(value.toStringList()),
                                    tr("Edit Frame Image List"),
                                    [](MT::MetaTileTilesetInput& ti) -> std::vector<std::string>& { return ti.animationFrames.frameImageFilenames; },
                                    [](MtTilesetResourceItem& item) { item.updateExternalFiles(); });

    case ANIMATION_DELAY:
        return undoHelper.editField(value.toUInt(),
                                    tr("Edit Animation Delay"),
                                    [](MT::MetaTileTilesetInput& ti) -> unsigned& { return ti.animationFrames.animationDelay; },
                                    [](MtTilesetResourceItem& item) { emit item.animationDelayChanged(); });

    case BIT_DEPTH:
        return undoHelper.editField(value.toUInt(),
                                    tr("Edit Bit Depth"),
                                    [](MT::MetaTileTilesetInput& ti) -> unsigned& { return ti.animationFrames.bitDepth; });

    case ADD_TRANSPARENT_TILE:
        return undoHelper.editField(value.toBool(),
                                    tr("Edit Add Transparent Tile"),
                                    [](MT::MetaTileTilesetInput& ti) -> bool& { return ti.animationFrames.addTransparentTile; });
    }

    return false;
}
