/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framecontentmanagers.h"
#include "accessors.h"
#include "document.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/metasprite/common.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite;
using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

const QStringList FrameObjectManager::SIZE_STRINGS({ QString::fromUtf8("Small"),
                                                     QString::fromUtf8("Large") });

const QStringList FrameObjectManager::FLIP_STRINGS({ QString(),
                                                     QString::fromUtf8("hFlip"),
                                                     QString::fromUtf8("vFlip"),
                                                     QString::fromUtf8("hvFlip") });

AbstractFrameContentManager::AbstractFrameContentManager(QObject* parent)
    : PropertyTableManager(parent)
    , _document(nullptr)
    , _frame(nullptr)
{
}

void AbstractFrameContentManager::setDocument(Document* document)
{
    if (_document) {
        _document->disconnect(this);
        _document->frameMap()->disconnect(this);
    }
    _document = document;

    _frame = nullptr;
    emit dataReset();

    if (_document) {
        onSelectedFrameChanged();

        connect(_document->frameMap(), &FrameMap::selectedItemChanged,
                this, &AbstractFrameContentManager::onSelectedFrameChanged);
    }
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
    const MS::Frame* frame = _document->frameMap()->selectedFrame();

    if (_frame != frame) {
        _frame = frame;
        emit dataReset();
    }
}

void AbstractFrameContentManager::onItemChanged(const void* frame, size_t index)
{
    if (frame == _frame) {
        emit itemChanged(index);
    }
}

void AbstractFrameContentManager::onListChanged(const void* frame)
{
    if (frame == _frame) {
        emit dataChanged();
    }
}

FrameObjectManager::FrameObjectManager(QObject* parent)
    : AbstractFrameContentManager(parent)
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
    if (_frame) {
        return _frame->objects.size();
    }
    else {
        return 0;
    }
}

QVariant FrameObjectManager::data(int index, int id) const
{
    using ObjSize = UnTech::MetaSprite::ObjectSize;

    if (_frame == nullptr
        || index < 0 || (unsigned)index >= _frame->objects.size()) {

        return QVariant();
    }

    const MS::FrameObject& obj = _frame->objects.at(index);

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

    const MS::FrameSet* frameSet = _document->frameSet();

    if (_frame == nullptr
        || frameSet == nullptr
        || index < 0 || (unsigned)index >= _frame->objects.size()) {

        return;
    }

    const MS::FrameObject& obj = _frame->objects.at(index);

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

    if (_frame == nullptr) {
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
    : AbstractFrameContentManager(parent)
{
    using Type = PropertyType;

    setTitle(tr("Action Point"));

    addProperty(tr("Location"), PropertyId::LOCATION, Type::POINT,
                QPoint(int_ms8_t::MIN, int_ms8_t::MIN), QPoint(int_ms8_t::MAX, int_ms8_t::MAX));
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
    if (_frame) {
        return _frame->actionPoints.size();
    }
    else {
        return 0;
    }
}

QVariant ActionPointManager::data(int index, int id) const
{
    if (_frame == nullptr
        || index < 0 || (unsigned)index >= _frame->actionPoints.size()) {

        return QVariant();
    }

    const MS::ActionPoint& ap = _frame->actionPoints.at(index);

    switch ((PropertyId)id) {
    case PropertyId::LOCATION:
        return fromMs8point(ap.location);

    case PropertyId::PARAMETER:
        return (int)ap.parameter;
    };

    return QVariant();
}

bool ActionPointManager::setData(int index, int id, const QVariant& value)
{
    if (_frame == nullptr) {
        return false;
    }

    switch ((PropertyId)id) {
    case PropertyId::LOCATION:
        return _document->actionPointList()->editSelectedList_setLocation(
            index, toMs8point(value.toPoint()));

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

    addProperty(tr("AABB"), PropertyId::AABB, Type::RECT,
                QRect(int_ms8_t::MIN, int_ms8_t::MIN, UINT8_MAX, UINT8_MAX));
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
    if (_frame) {
        return _frame->entityHitboxes.size();
    }
    else {
        return 0;
    }
}

QVariant EntityHitboxManager::data(int index, int id) const
{
    if (_frame == nullptr
        || index < 0 || (unsigned)index >= _frame->entityHitboxes.size()) {

        return QVariant();
    }

    const MS::EntityHitbox& eh = _frame->entityHitboxes.at(index);

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

    if (_frame == nullptr) {
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
