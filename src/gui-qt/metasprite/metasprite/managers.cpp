/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "managers.h"
#include "accessors.h"
#include "document.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/metasprite/actionpoints/actionpointsresourceitem.h"
#include "gui-qt/metasprite/common.h"
#include "gui-qt/metasprite/exportorder/exportorderresourcelist.h"
#include "gui-qt/project.h"
#include "gui-qt/staticresourcelist.h"

using namespace UnTech;
using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite;
using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

constexpr QRect MS8_RECT_BOUNDS(int_ms8_t::MIN, int_ms8_t::MIN, UINT8_MAX, UINT8_MAX);

const QStringList FrameObjectManager::SIZE_STRINGS({ QString::fromUtf8("Small"),
                                                     QString::fromUtf8("Large") });

const QStringList FrameObjectManager::FLIP_STRINGS({ QString(),
                                                     QString::fromUtf8("hFlip"),
                                                     QString::fromUtf8("vFlip"),
                                                     QString::fromUtf8("hvFlip") });

FrameSetManager::FrameSetManager(QObject* parent)
    : PropertyListManager(parent)
    , _document(nullptr)
{
    using Type = UnTech::GuiQt::PropertyType;

    addProperty(tr("Name"), NAME, Type::IDSTRING);
    addProperty(tr("Tileset Type"), TILESET_TYPE, Type::COMBO, TILESET_TYPE_STRINGS, TILESET_TYPE_VALUES);
    addProperty(tr("Export Order"), EXPORT_ORDER, Type::COMBO);
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

    const MS::FrameSet* frameSet = _document->frameSet();
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
    }

    return QVariant();
}

void FrameSetManager::updateParameters(int id, QVariant& param1, QVariant&) const
{
    if (_document == nullptr) {
        return;
    }

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
    case PropertyId::TILESET_TYPE:
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

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
        return _document->editFrameSet_setName(value.toString().toStdString());

    case PropertyId::TILESET_TYPE:
        return _document->editFrameSet_setTilesetType(static_cast<TtEnum>(value.toInt()));

    case PropertyId::EXPORT_ORDER:
        return _document->editFrameSet_setExportOrder(value.toString().toStdString());
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
    addPropertyGroup(tr("Tile Hitbox:"));
    addProperty(tr("Solid"), SOLID, Type::BOOLEAN);
    addProperty(tr("AABB"), TILE_HITBOX, Type::RECT, MS8_RECT_BOUNDS);
}

void FrameManager::setDocument(Document* document)
{
    auto* frameList = document ? document->frameList() : nullptr;

    if (_frameList) {
        _frameList->disconnect(this);
    }
    _frameList = frameList;

    if (_frameList) {
        connect(_frameList, &FrameList::selectedIndexChanged,
                this, &FrameManager::onSelectedFrameChanged);
        connect(_frameList, &FrameList::dataChanged,
                this, &FrameManager::onFrameDataChanged);
        connect(_frameList, &FrameList::listAboutToChange,
                this, &FrameManager::listAboutToChange);
    }

    onSelectedFrameChanged();
}

void FrameManager::onSelectedFrameChanged()
{
    setEnabled(_frameList && _frameList->isSelectedIndexValid());
    emit dataChanged();
}

void FrameManager::onFrameDataChanged(size_t frameIndex)
{
    Q_ASSERT(_frameList);
    if (frameIndex == _frameList->selectedIndex()) {
        emit dataChanged();
    }
}

QVariant FrameManager::data(int id) const
{
    if (_frameList == nullptr) {
        return QVariant();
    }

    const MS::Frame* frame = _frameList->selectedItem();
    if (frame == nullptr) {
        return QVariant();
    }

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
        return QString::fromStdString(frame->name);

    case PropertyId::SPRITE_ORDER:
        return unsigned(frame->spriteOrder);

    case PropertyId::SOLID:
        return frame->solid;

    case PropertyId::TILE_HITBOX:
        if (frame->solid) {
            return fromMs8rect(frame->tileHitbox);
        }
    }

    return QVariant();
}

bool FrameManager::setData(int id, const QVariant& value)
{
    if (_frameList == nullptr) {
        return false;
    }

    const MS::Frame* frame = _frameList->selectedItem();
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
        return _frameList->editSelected_setTileHitbox(toMs8rect(value.toRect()));
    }

    return false;
}

FrameObjectManager::FrameObjectManager(QObject* parent)
    : ListAccessorTableManager(parent)
{
    using Type = PropertyType;

    setTitle(tr("Frame Objects"));

    addProperty(tr("Location"), PropertyId::LOCATION, Type::POINT,
                QPoint(int_ms8_t::MIN, int_ms8_t::MIN), QPoint(int_ms8_t::MAX, int_ms8_t::MAX));
    addProperty(tr("Size"), PropertyId::SIZE, Type::COMBO, SIZE_STRINGS, QVariantList{ false, true });
    addProperty(tr("Tile"), PropertyId::TILE, Type::UNSIGNED, 0, 0);
    addProperty(tr("Flip"), PropertyId::FLIP, Type::COMBO, FLIP_STRINGS, QVariantList{ 0, 1, 2, 3 });
}

void FrameObjectManager::setDocument(Document* document)
{
    _document = document;
    setAccessor(document ? document->frameObjectList() : nullptr);
}

const MS::Frame* FrameObjectManager::selectedFrame() const
{
    return _document ? _document->frameList()->selectedItem() : nullptr;
}

QVariant FrameObjectManager::data(int index, int id) const
{
    using ObjSize = UnTech::MetaSprite::ObjectSize;

    const MS::Frame* frame = selectedFrame();
    if (frame == nullptr
        || index < 0 || (unsigned)index >= frame->objects.size()) {

        return QVariant();
    }

    const MS::FrameObject& obj = frame->objects.at(index);

    switch ((PropertyId)id) {
    case PropertyId::LOCATION:
        return fromMs8point(obj.location);

    case PropertyId::SIZE:
        return obj.size == ObjSize::LARGE;

    case PropertyId::TILE:
        return obj.tileId;

    case PropertyId::FLIP:
        return (obj.vFlip << 1) | obj.hFlip;
    };

    return QVariant();
}

void FrameObjectManager::updateParameters(int index, int id,
                                          QVariant& param1, QVariant& param2) const
{
    using ObjSize = UnTech::MetaSprite::ObjectSize;

    const MS::Frame* frame = selectedFrame();
    if (frame == nullptr
        || index < 0 || (unsigned)index >= frame->objects.size()) {

        return;
    }

    const MS::FrameSet* frameSet = _document->frameSet();
    Q_ASSERT(frameSet);

    const MS::FrameObject& obj = frame->objects.at(index);

    switch ((PropertyId)id) {
    case PropertyId::TILE:
        param1 = 0;
        param2 = obj.size == ObjSize::SMALL
                     ? unsigned(frameSet->smallTileset.size() - 1)
                     : unsigned(frameSet->largeTileset.size() - 1);
        break;

    case PropertyId::LOCATION:
    case PropertyId::SIZE:
    case PropertyId::FLIP:
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
            index, toMs8point(value.toPoint()));

    case PropertyId::SIZE:
        return _document->frameObjectList()->editSelectedList_setSize(
            index, value.toBool() ? ObjSize::LARGE : ObjSize::SMALL);

    case PropertyId::TILE:
        return _document->frameObjectList()->editSelectedList_setTile(
            index, value.toUInt());

    case PropertyId::FLIP:
        return _document->frameObjectList()->editSelectedList_setFlips(
            index, value.toUInt() & 1, value.toUInt() & 2);
    };

    return false;
}

ActionPointManager::ActionPointManager(QObject* parent)
    : ListAccessorTableManager(parent)
{
    using Type = PropertyType;

    setTitle(tr("Action Point"));

    addProperty(tr("Location"), PropertyId::LOCATION, Type::POINT,
                QPoint(int_ms8_t::MIN, int_ms8_t::MIN), QPoint(int_ms8_t::MAX, int_ms8_t::MAX));
    addProperty(tr("Type"), PropertyId::TYPE, Type::COMBO);
}

void ActionPointManager::setDocument(Document* document)
{
    _document = document;
    setAccessor(document ? document->actionPointList() : nullptr);
}

const MS::Frame* ActionPointManager::selectedFrame() const
{
    return _document ? _document->frameList()->selectedItem() : nullptr;
}

QVariant ActionPointManager::data(int index, int id) const
{
    const MS::Frame* frame = selectedFrame();
    if (frame == nullptr
        || index < 0 || (unsigned)index >= frame->actionPoints.size()) {

        return QVariant();
    }

    const MS::ActionPoint& ap = frame->actionPoints.at(index);

    switch ((PropertyId)id) {
    case PropertyId::LOCATION:
        return fromMs8point(ap.location);

    case PropertyId::TYPE:
        return QString::fromStdString(ap.type);
    };

    return QVariant();
}

void ActionPointManager::updateParameters(int index, int id, QVariant& param1, QVariant&) const
{
    if (_document == nullptr) {
        return;
    }

    Q_UNUSED(index);

    if (id == PropertyId::TYPE) {
        param1 = _document->project()->staticResourceList()->actionPointsResourceItem()->actionPointNames();
    }
}

bool ActionPointManager::setData(int index, int id, const QVariant& value)
{
    if (_document == nullptr) {
        return false;
    }

    switch ((PropertyId)id) {
    case PropertyId::LOCATION:
        return _document->actionPointList()->editSelectedList_setLocation(
            index, toMs8point(value.toPoint()));

    case PropertyId::TYPE:
        return _document->actionPointList()->editSelectedList_setType(
            index, value.toString().toStdString());
    };

    return false;
}

EntityHitboxManager::EntityHitboxManager(QObject* parent)
    : ListAccessorTableManager(parent)
{
    using Type = PropertyType;

    setTitle(tr("Entity Hitbox"));

    addProperty(tr("AABB"), PropertyId::AABB, Type::RECT, MS8_RECT_BOUNDS);
    addProperty(tr("Hitbox Type"), PropertyId::HITBOX_TYPE, Type::COMBO,
                EH_SHORT_STRING_VALUES, qVariantRange(EH_SHORT_STRING_VALUES.size()));
}

void EntityHitboxManager::setDocument(Document* document)
{
    _document = document;
    setAccessor(document ? document->entityHitboxList() : nullptr);
}

const MS::Frame* EntityHitboxManager::selectedFrame() const
{
    return _document ? _document->frameList()->selectedItem() : nullptr;
}

QVariant EntityHitboxManager::data(int index, int id) const
{
    const MS::Frame* frame = selectedFrame();
    if (frame == nullptr
        || index < 0 || (unsigned)index >= frame->entityHitboxes.size()) {

        return QVariant();
    }

    const MS::EntityHitbox& eh = frame->entityHitboxes.at(index);

    switch ((PropertyId)id) {
    case PropertyId::AABB:
        return fromMs8rect(eh.aabb);

    case PropertyId::HITBOX_TYPE:
        return int(eh.hitboxType.romValue());
    };

    return QVariant();
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
            index, toMs8rect(value.toRect()));

    case PropertyId::HITBOX_TYPE:
        return _document->entityHitboxList()->editSelectedList_setEntityHitboxType(
            index, EntityHitboxType::from_romValue(value.toInt()));
    };

    return false;
}
