/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "managers.h"
#include "accessors.h"
#include "document.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/metasprite/common.h"
#include "gui-qt/metasprite/exportorder/exportorderresourcelist.h"
#include "gui-qt/project.h"
#include "models/common/imagecache.h"

using namespace UnTech::GuiQt::MetaSprite;
using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

const QStringList FrameObjectManager::SIZE_STRINGS({ QString::fromUtf8("Small"),
                                                     QString::fromUtf8("Large") });

FrameSetManager::FrameSetManager(QObject* parent)
    : PropertyListManager(parent)
    , _document(nullptr)
{
    using Type = UnTech::GuiQt::PropertyType;

    addProperty(tr("Name"), NAME, Type::IDSTRING);
    addProperty(tr("Tileset Type"), TILESET_TYPE, Type::COMBO, TILESET_TYPE_STRINGS, TILESET_TYPE_VALUES);
    addProperty(tr("Export Order"), EXPORT_ORDER, Type::COMBO);
    addPropertyGroup(tr("Image:"));
    addProperty(tr("Filename"), IMAGE_FILENAME, Type::FILENAME, QStringLiteral("PNG Image (*.png)"));
    addProperty(tr("Transparent Color"), TRANSPARENT_COLOR, Type::COLOR);
    addPropertyGroup(tr("Grid:"));
    addProperty(tr("Frame Size"), GRID_FRAME_SIZE, Type::SIZE);
    addProperty(tr("Offset"), GRID_OFFSET, Type::POINT);
    addProperty(tr("Padding"), GRID_PADDING, Type::POINT);
    addProperty(tr("Origin"), GRID_ORIGIN, Type::POINT);
    addPropertyGroup(tr("Palette:"));
    addProperty(tr("User Supplied Palette"), USER_SUPPLIED_PALETTE, Type::BOOLEAN);
    addProperty(tr("No of Palettes"), PALETTE_N_PALLETES, Type::UNSIGNED, 0, unsigned(UnTech::MetaSprite::MAX_PALETTES));
    addProperty(tr("Color Size"), PALETTE_COLOR_SIZE, Type::UNSIGNED, 0, 32);
}

void FrameSetManager::setDocument(Document* document)
{
    if (_document) {
        _document->disconnect(this);
    }
    _document = document;

    if (_document) {
        connect(_document, &Document::nameChanged,
                this, &FrameManager::dataChanged);
        connect(_document, &Document::frameSetDataChanged,
                this, &FrameManager::dataChanged);
    }

    setEnabled(_document != nullptr);
    emit dataChanged();
}

QVariant FrameSetManager::data(int id) const
{
    if (_document == nullptr) {
        return QVariant();
    }

    const SI::FrameSet* frameSet = _document->frameSet();
    if (frameSet == nullptr) {
        return QVariant();
    }

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
        return QString::fromStdString(frameSet->name);

    case PropertyId::TILESET_TYPE:
        return int(frameSet->tilesetType.value());

    case PropertyId::EXPORT_ORDER:
        return QString::fromStdString(frameSet->exportOrder);

    case IMAGE_FILENAME:
        return QString::fromStdString(frameSet->imageFilename);

    case TRANSPARENT_COLOR:
        return fromRgba(frameSet->transparentColor);

    case GRID_FRAME_SIZE:
        return fromUsize(frameSet->grid.frameSize);

    case GRID_OFFSET:
        return fromUpoint(frameSet->grid.offset);

    case GRID_PADDING:
        return fromUpoint(frameSet->grid.padding);

    case GRID_ORIGIN:
        return fromUpoint(frameSet->grid.origin);

    case USER_SUPPLIED_PALETTE:
        return frameSet->palette.usesUserSuppliedPalette();

    case PALETTE_N_PALLETES:
        if (frameSet->palette.usesUserSuppliedPalette()) {
            return frameSet->palette.nPalettes;
        }
        else {
            return 0;
        }

    case PALETTE_COLOR_SIZE:
        if (frameSet->palette.usesUserSuppliedPalette()) {
            return frameSet->palette.colorSize;
        }
        else {
            return 0;
        }
    }

    return QVariant();
}

void FrameSetManager::updateParameters(int id, QVariant& param1, QVariant& param2) const
{
    if (_document == nullptr) {
        return;
    }

    const SI::FrameSet* frameSet = _document->frameSet();
    if (frameSet == nullptr) {
        return;
    }

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
    case PropertyId::TILESET_TYPE:
    case IMAGE_FILENAME:
    case TRANSPARENT_COLOR:
    case GRID_FRAME_SIZE:
    case GRID_OFFSET:
    case GRID_PADDING:
    case USER_SUPPLIED_PALETTE:
    case PALETTE_N_PALLETES:
    case PALETTE_COLOR_SIZE:
        break;

    case GRID_ORIGIN:
        param2 = QPoint(frameSet->grid.originRange().width, frameSet->grid.originRange().height);
        break;

    case PropertyId::EXPORT_ORDER:
        param1 = _document->project()->frameSetExportOrderResourceList()->itemNames();
    }
}

bool FrameSetManager::setData(int id, const QVariant& value)
{
    using TtEnum = UnTech::MetaSprite::TilesetType::Enum;

    if (_document == nullptr) {
        return false;
    }

    const SI::FrameSet* frameSet = _document->frameSet();
    if (frameSet == nullptr) {
        return false;
    }

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
        return _document->editFrameSet_setName(value.toString().toStdString());

    case PropertyId::TILESET_TYPE:
        return _document->editFrameSet_setTilesetType(static_cast<TtEnum>(value.toInt()));

    case PropertyId::EXPORT_ORDER:
        return _document->editFrameSet_setExportOrder(value.toString().toStdString());

    case IMAGE_FILENAME:
        return _document->editFrameSet_setImageFilename(value.toString().toStdString());

    case TRANSPARENT_COLOR:
        return _document->editFrameSet_setTransparentColor(toRgba(value.value<QColor>()));

    case GRID_FRAME_SIZE:
    case GRID_OFFSET:
    case GRID_PADDING:
    case GRID_ORIGIN: {
        SI::FrameSetGrid grid = frameSet->grid;

        switch (static_cast<PropertyId>(id)) {
        default:
            break;

        case GRID_FRAME_SIZE:
            grid.frameSize = toUsize(value.toSize());
            break;

        case GRID_OFFSET:
            grid.offset = toUpoint(value.toPoint());
            break;

        case GRID_PADDING:
            grid.padding = toUpoint(value.toPoint());
            break;

        case GRID_ORIGIN:
            grid.origin = toUpoint(value.toPoint());

            break;
        }

        return _document->editFrameSet_setGrid(grid);
    }

    case USER_SUPPLIED_PALETTE:
    case PALETTE_N_PALLETES:
    case PALETTE_COLOR_SIZE: {
        SI::UserSuppliedPalette palette = frameSet->palette;

        switch (static_cast<PropertyId>(id)) {
        default:
            break;

        case USER_SUPPLIED_PALETTE:
            if (value == true) {
                if (palette.nPalettes == 0) {
                    palette.nPalettes = 1;
                }
                if (palette.colorSize == 0) {
                    palette.colorSize = 4;
                }
            }
            else {
                palette.nPalettes = 0;
            }
            break;

        case PALETTE_N_PALLETES:
            palette.nPalettes = value.toUInt();
            break;

        case PALETTE_COLOR_SIZE:
            palette.colorSize = value.toUInt();
            break;
        }

        return _document->editFrameSet_setPalette(palette);
    }
    }

    return false;
}

FrameManager::FrameManager(QObject* parent)
    : PropertyListManager(parent)
    , _frameList(nullptr)
{
    using Type = UnTech::GuiQt::PropertyType;

    addProperty(tr("Name"), NAME, Type::IDSTRING);
    addProperty(tr("Sprite Order"), SPRITE_ORDER, Type::UNSIGNED, 0, 3);
    addPropertyGroup(tr("Location:"));
    addProperty(tr("Use Grid Location"), LOCATION_USE_GRID_LOCATION, Type::BOOLEAN);
    addProperty(tr("Grid Location"), LOCATION_GRID_LOCATION, Type::POINT);
    addProperty(tr("AABB"), LOCATION_AABB, Type::RECT);
    addProperty(tr("Use Grid Origin"), LOCATION_USE_GRID_ORGIN, Type::BOOLEAN);
    addProperty(tr("Origin"), LOCATION_ORIGIN, Type::POINT);
    addPropertyGroup(tr("Tile Hitbox:"));
    addProperty(tr("Solid"), SOLID, Type::BOOLEAN);
    addProperty(tr("AABB"), TILE_HITBOX, Type::RECT);
}

void FrameManager::setDocument(Document* document)
{
    auto* frameList = document ? document->frameList() : nullptr;

    if (_frameList) {
        _frameList->disconnect(this);
    }
    _frameList = frameList;

    onSelectedFrameChanged();

    if (_frameList) {
        connect(_frameList, &FrameList::selectedIndexChanged,
                this, &FrameManager::onSelectedFrameChanged);
        connect(_frameList, &FrameList::dataChanged,
                this, &FrameManager::onFrameDataChanged);
        connect(_frameList, &FrameList::listAboutToChange,
                this, &FrameManager::listAboutToChange);
    }

    emit dataChanged();
}

void FrameManager::onSelectedFrameChanged()
{
    setEnabled(_frameList && _frameList->isSelectedIndexValid());
    dataChanged();
}

void FrameManager::onFrameDataChanged(size_t frameIndex)
{
    Q_ASSERT(_frameList);
    if (frameIndex == _frameList->selectedIndex()) {
        dataChanged();
    }
}

QVariant FrameManager::data(int id) const
{
    if (_frameList == nullptr) {
        return QVariant();
    }

    const SI::Frame* frame = _frameList->selectedItem();
    if (frame == nullptr) {
        return QVariant();
    }

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
        return QString::fromStdString(frame->name);

    case PropertyId::SPRITE_ORDER:
        return unsigned(frame->spriteOrder);

    case PropertyId::LOCATION_USE_GRID_LOCATION:
        return frame->location.useGridLocation;

    case PropertyId::LOCATION_GRID_LOCATION:
        return fromUpoint(frame->location.gridLocation);

    case PropertyId::LOCATION_AABB:
        return fromUrect(frame->location.aabb);

    case PropertyId::LOCATION_USE_GRID_ORGIN:
        return frame->location.useGridOrigin;

    case PropertyId::LOCATION_ORIGIN:
        return fromUpoint(frame->location.origin);

    case PropertyId::SOLID:
        return frame->solid;

    case PropertyId::TILE_HITBOX:
        if (frame->solid) {
            return fromUrect(frame->tileHitbox);
        }
    }

    return QVariant();
}

void FrameManager::updateParameters(int id, QVariant& param1, QVariant& param2) const
{
    if (_frameList == nullptr) {
        return;
    }

    const SI::Frame* frame = _frameList->selectedItem();
    if (frame == nullptr) {
        return;
    }

    const SI::FrameSet* frameSet = _frameList->resourceItem()->frameSet();
    Q_ASSERT(frameSet);
    auto getImageSize = [&]() -> usize {
        const auto image = ImageCache::loadPngImage(frameSet->imageFilename);
        Q_ASSERT(image);
        return image->size();
    };

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
    case PropertyId::SPRITE_ORDER:
    case PropertyId::LOCATION_USE_GRID_LOCATION:
    case PropertyId::LOCATION_USE_GRID_ORGIN:
    case PropertyId::SOLID:
        break;

    case PropertyId::LOCATION_GRID_LOCATION: {
        usize imageSize = getImageSize();
        auto& fWidth = frameSet->grid.frameSize.width;
        auto& fHeight = frameSet->grid.frameSize.height;

        param1 = QPoint(0, 0);
        if (imageSize.width > fWidth && imageSize.height > fHeight) {
            param2 = QPoint(imageSize.width / fWidth - 1,
                            imageSize.height / fHeight - 1);
        }
    } break;

    case PropertyId::LOCATION_AABB: {
        usize imageSize = getImageSize();
        param1 = QRect(0, 0, imageSize.width, imageSize.height);
        param2 = QSize(256, 256);
    } break;

    case PropertyId::LOCATION_ORIGIN: {
        const urect& aabb = frame->location.aabb;

        param1 = QPoint(0, 0);
        param2 = QPoint(aabb.width - 1, aabb.height - 1);
    } break;

    case PropertyId::TILE_HITBOX: {
        const urect& aabb = frame->location.aabb;

        param1 = QRect(0, 0, aabb.width, aabb.height);
    } break;
    }
}

bool FrameManager::setData(int id, const QVariant& value)
{
    if (_frameList == nullptr) {
        return false;
    }

    const SI::Frame* frame = _frameList->selectedItem();
    if (frame == nullptr) {
        return false;
    }

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
        return _frameList->editSelected_setName(value.toString().toStdString());

    case PropertyId::SPRITE_ORDER:
        return _frameList->editSelected_setSpriteOrder(value.toUInt());

    case PropertyId::SOLID:
        return _frameList->editSelected_setSolid(value.toBool());

    case PropertyId::TILE_HITBOX:
        return _frameList->editSelected_setTileHitbox(toUrect(value.toRect()));

    case PropertyId::LOCATION_USE_GRID_LOCATION:
    case PropertyId::LOCATION_GRID_LOCATION:
    case PropertyId::LOCATION_AABB:
    case PropertyId::LOCATION_USE_GRID_ORGIN:
    case PropertyId::LOCATION_ORIGIN: {
        const SI::FrameSet* frameSet = _frameList->resourceItem()->frameSet();
        Q_ASSERT(frameSet);

        SI::FrameLocation frameLocation = frame->location;

        switch (static_cast<PropertyId>(id)) {
        default:
            break;

        case PropertyId::LOCATION_USE_GRID_LOCATION:
            frameLocation.useGridLocation = value.toBool();
            break;

        case PropertyId::LOCATION_GRID_LOCATION:
            frameLocation.useGridLocation = true;
            frameLocation.gridLocation = toUpoint(value.toPoint());
            break;

        case PropertyId::LOCATION_AABB:
            frameLocation.useGridLocation = false;
            frameLocation.aabb = toUrect(value.toRect());
            break;

        case PropertyId::LOCATION_USE_GRID_ORGIN:
            frameLocation.useGridOrigin = value.toBool();
            break;

        case PropertyId::LOCATION_ORIGIN:
            frameLocation.useGridOrigin = false;
            frameLocation.origin = toUpoint(value.toPoint());
            break;
        }

        frameLocation.update(frameSet->grid, *frame);
        return _frameList->editSelected_setFrameLocation(frameLocation);
    }
    }

    return false;
}

AbstractFrameContentManager::AbstractFrameContentManager(QObject* parent)
    : PropertyTableManager(parent)
    , _document(nullptr)
{
}

void AbstractFrameContentManager::setDocument(Document* document)
{
    if (_document) {
        _document->frameList()->disconnect(this);
    }
    _document = document;

    emit dataReset();

    if (_document) {
        onSelectedFrameChanged();

        connect(_document->frameList(), &FrameList::selectedIndexChanged,
                this, &AbstractFrameContentManager::onSelectedFrameChanged);

        connect(_document->frameList(), &FrameList::frameLocationChanged,
                this, &AbstractFrameContentManager::onFrameLocationChanged);
    }
}

const SI::Frame* AbstractFrameContentManager::selectedFrame() const
{
    if (_document == nullptr) {
        return nullptr;
    }
    return _document->frameList()->selectedItem();
}

void AbstractFrameContentManager::connectSignals(AbstractFrameContentAccessor* accessor)
{
    connect(accessor, &AbstractFrameContentAccessor::dataChanged,
            this, &EntityHitboxManager::onItemChanged);

    connect(accessor, &AbstractFrameContentAccessor::listAboutToChange,
            this, &AbstractFrameContentManager::listAboutToChange);

    // Use listChanged instead of add/remove to prevent QItemSelectionModel
    // from corrupting the accessor selectedIndexes.
    connect(accessor, &AbstractFrameContentAccessor::listChanged,
            this, &EntityHitboxManager::onListChanged);
}

void AbstractFrameContentManager::onSelectedFrameChanged()
{
    dataReset();
}

void AbstractFrameContentManager::onFrameLocationChanged(size_t frameIndex)
{
    Q_ASSERT(_document);
    if (frameIndex == _document->frameList()->selectedIndex()) {
        emit dataChanged();
    }
}

void AbstractFrameContentManager::onItemChanged(size_t frameIndex, size_t index)
{
    Q_ASSERT(_document);
    if (frameIndex == _document->frameList()->selectedIndex()) {
        emit itemChanged(index);
    }
}

void AbstractFrameContentManager::onListChanged(size_t frameIndex)
{
    Q_ASSERT(_document);
    if (frameIndex == _document->frameList()->selectedIndex()) {
        emit dataChanged();
    }
}

FrameObjectManager::FrameObjectManager(QObject* parent)
    : AbstractFrameContentManager(parent)
{
    using Type = PropertyType;

    setTitle(tr("Frame Objects"));

    addProperty(tr("Location"), PropertyId::LOCATION, Type::POINT);
    addProperty(tr("Size"), PropertyId::SIZE, Type::COMBO, SIZE_STRINGS, QVariantList{ false, true });
}

void FrameObjectManager::setDocument(Document* document)
{
    if (_document) {
        _document->frameObjectList()->disconnect(this);
    }

    AbstractFrameContentManager::setDocument(document);

    if (document) {
        connectSignals(document->frameObjectList());
    }
}

int FrameObjectManager::rowCount() const
{
    const SI::Frame* frame = selectedFrame();
    if (frame) {
        return frame->objects.size();
    }
    else {
        return 0;
    }
}

QVariant FrameObjectManager::data(int index, int id) const
{
    using ObjSize = UnTech::MetaSprite::ObjectSize;

    const SI::Frame* frame = selectedFrame();
    if (frame == nullptr
        || index < 0 || (unsigned)index >= frame->objects.size()) {

        return QVariant();
    }

    const SI::FrameObject& obj = frame->objects.at(index);

    switch ((PropertyId)id) {
    case PropertyId::LOCATION:
        return fromUpoint(obj.location);

    case PropertyId::SIZE:
        return obj.size == ObjSize::LARGE;
    };

    return QVariant();
}

void FrameObjectManager::updateParameters(int index, int id,
                                          QVariant& param1, QVariant& param2) const
{
    const SI::Frame* frame = selectedFrame();
    if (frame == nullptr
        || index < 0 || (unsigned)index >= frame->objects.size()) {

        return;
    }

    const SI::FrameObject& obj = frame->objects.at(index);

    switch ((PropertyId)id) {
    case PropertyId::LOCATION: {
        const usize s = frame->location.aabb.size();
        const unsigned o = obj.sizePx();

        param1 = QPoint(0, 0);
        param2 = QPoint(s.width - o, s.height - o);
    } break;

    case PropertyId::SIZE:
        break;
    };
}

bool FrameObjectManager::setData(int index, int id, const QVariant& value)
{
    using ObjSize = UnTech::MetaSprite::ObjectSize;

    if (_document == nullptr) {
        return false;
    }

    switch ((PropertyId)id) {
    case PropertyId::LOCATION:
        return _document->frameObjectList()->editSelectedList_setLocation(
            index, toUpoint(value.toPoint()));

    case PropertyId::SIZE:
        return _document->frameObjectList()->editSelectedList_setSize(
            index, value.toBool() ? ObjSize::LARGE : ObjSize::SMALL);
    };

    return false;
}

ActionPointManager::ActionPointManager(QObject* parent)
    : AbstractFrameContentManager(parent)
{
    using Type = PropertyType;

    setTitle(tr("Action Point"));

    addProperty(tr("Location"), PropertyId::LOCATION, Type::POINT);
    addProperty(tr("Parameter"), PropertyId::PARAMETER, Type::UNSIGNED, 0, 255);
}

void ActionPointManager::setDocument(Document* document)
{
    if (_document) {
        _document->actionPointList()->disconnect(this);
    }

    AbstractFrameContentManager::setDocument(document);

    if (document) {
        connectSignals(document->actionPointList());
    }
}

int ActionPointManager::rowCount() const
{
    const SI::Frame* frame = selectedFrame();
    if (frame) {
        return frame->actionPoints.size();
    }
    else {
        return 0;
    }
}

QVariant ActionPointManager::data(int index, int id) const
{
    const SI::Frame* frame = selectedFrame();
    if (frame == nullptr
        || index < 0 || (unsigned)index >= frame->actionPoints.size()) {

        return QVariant();
    }

    const SI::ActionPoint& ap = frame->actionPoints.at(index);

    switch ((PropertyId)id) {
    case PropertyId::LOCATION:
        return fromUpoint(ap.location);

    case PropertyId::PARAMETER:
        return (int)ap.parameter;
    };

    return QVariant();
}

void ActionPointManager::updateParameters(int index, int id,
                                          QVariant& param1, QVariant& param2) const
{
    const SI::Frame* frame = selectedFrame();
    if (frame == nullptr
        || index < 0 || (unsigned)index >= frame->actionPoints.size()) {

        return;
    }

    switch ((PropertyId)id) {
    case PropertyId::LOCATION: {
        const usize s = frame->location.aabb.size();

        param1 = QPoint(0, 0);
        param2 = QPoint(s.width - 1, s.height - 1);
    } break;

    case PropertyId::PARAMETER:
        break;
    };
}

bool ActionPointManager::setData(int index, int id, const QVariant& value)
{
    if (_document == nullptr) {
        return false;
    }

    switch ((PropertyId)id) {
    case PropertyId::LOCATION:
        return _document->actionPointList()->editSelectedList_setLocation(
            index, toUpoint(value.toPoint()));

    case PropertyId::PARAMETER:
        return _document->actionPointList()->editSelectedList_setParameter(
            index, value.toUInt());
    };

    return false;
}

EntityHitboxManager::EntityHitboxManager(QObject* parent)
    : AbstractFrameContentManager(parent)
{
    using Type = PropertyType;

    setTitle(tr("Entity Hitbox"));

    addProperty(tr("AABB"), PropertyId::AABB, Type::RECT);
    addProperty(tr("Hitbox Type"), PropertyId::HITBOX_TYPE, Type::COMBO,
                EH_SHORT_STRING_VALUES, qVariantRange(EH_SHORT_STRING_VALUES.size()));
}

void EntityHitboxManager::setDocument(Document* document)
{
    if (_document) {
        _document->entityHitboxList()->disconnect(this);
    }

    AbstractFrameContentManager::setDocument(document);

    if (document) {
        connectSignals(document->entityHitboxList());
    }
}

int EntityHitboxManager::rowCount() const
{
    const SI::Frame* frame = selectedFrame();
    if (frame) {
        return frame->entityHitboxes.size();
    }
    else {
        return 0;
    }
}

QVariant EntityHitboxManager::data(int index, int id) const
{
    const SI::Frame* frame = selectedFrame();
    if (frame == nullptr
        || index < 0 || (unsigned)index >= frame->entityHitboxes.size()) {

        return QVariant();
    }

    const SI::EntityHitbox& eh = frame->entityHitboxes.at(index);

    switch ((PropertyId)id) {
    case PropertyId::AABB:
        return fromUrect(eh.aabb);

    case PropertyId::HITBOX_TYPE:
        return int(eh.hitboxType.romValue());
    };

    return QVariant();
}

void EntityHitboxManager::updateParameters(int index, int id,
                                           QVariant& param1, QVariant& param2) const
{
    const SI::Frame* frame = selectedFrame();
    if (frame == nullptr
        || index < 0 || (unsigned)index >= frame->entityHitboxes.size()) {

        return;
    }

    Q_UNUSED(param2);

    switch ((PropertyId)id) {
    case PropertyId::AABB: {
        const usize s = frame->location.aabb.size();
        param1 = QRect(0, 0, s.width, s.height);
    } break;

    case PropertyId::HITBOX_TYPE:
        break;
    };
}

bool EntityHitboxManager::setData(int index, int id, const QVariant& value)
{
    using EntityHitboxType = UnTech::MetaSprite::EntityHitboxType;

    if (_document == nullptr) {
        return false;
    }

    switch ((PropertyId)id) {
    case PropertyId::AABB:
        return _document->entityHitboxList()->editSelectedList_setAabb(
            index, toUrect(value.toRect()));

    case PropertyId::HITBOX_TYPE:
        return _document->entityHitboxList()->editSelectedList_setEntityHitboxType(
            index, EntityHitboxType::from_romValue(value.toInt()));
    };

    return false;
}
