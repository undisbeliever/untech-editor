/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framecontentmanagers.h"
#include "document.h"
#include "framecontentcommands.h"
#include "gui-qt/common/helpers.h"

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
        _document->selection()->disconnect(this);
    }
    _document = document;

    _frame = nullptr;
    dataReset();

    if (_document) {
        onSelectedFrameChanged();

        connect(_document->selection(), &Selection::selectedFrameChanged,
                this, &AbstractFrameContentManager::onSelectedFrameChanged);
    }
}

void AbstractFrameContentManager::onSelectedFrameChanged()
{
    MS::Frame* frame = _document->selection()->selectedFrame();

    if (_frame != frame) {
        _frame = frame;
        dataReset();
    }
}

void AbstractFrameContentManager::onItemChanged(const void* frame, unsigned index)
{
    if (frame == _frame) {
        emit itemChanged(index);
    }
}

void AbstractFrameContentManager::onItemAdded(const void* frame, unsigned index)
{
    if (frame == _frame) {
        emit itemAdded(index);
    }
}

void AbstractFrameContentManager::onItemAboutToBeRemoved(const void* frame, unsigned index)
{
    if (frame == _frame) {
        emit itemRemoved(index);
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
    AbstractFrameContentManager::setDocument(document);

    if (_document) {
        connect(_document, &Document::frameObjectChanged,
                this, &FrameObjectManager::onItemChanged);
        connect(_document, &Document::frameObjectAdded,
                this, &FrameObjectManager::onItemAdded);
        connect(_document, &Document::frameObjectAboutToBeRemoved,
                this, &FrameObjectManager::onItemAboutToBeRemoved);
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
        return QPoint(obj.location.x, obj.location.y);

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

    if (_frame == nullptr
        || index < 0 || (unsigned)index >= _frame->objects.size()) {

        return false;
    }

    const MS::FrameObject& oldObj = _frame->objects.at(index);
    MS::FrameObject obj = oldObj;

    switch ((PropertyId)id) {
    case PropertyId::LOCATION:
        obj.location.x = value.toPoint().x();
        obj.location.y = value.toPoint().y();
        break;

    case PropertyId::SIZE:
        obj.size = value.toBool() ? ObjSize::LARGE : ObjSize::SMALL;
        break;

    case PropertyId::TILE:
        obj.tileId = value.toUInt();
        break;

    case PropertyId::FLIP:
        obj.hFlip = value.toUInt() & 1;
        obj.vFlip = value.toUInt() & 2;
        break;
    };

    if (obj != oldObj) {
        _document->undoStack()->push(
            new ChangeFrameObject(_document, _frame, index, obj));

        return true;
    }
    return false;
}

SelectedItem::Type FrameObjectManager::itemType() const
{
    return SelectedItem::FRAME_OBJECT;
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
    AbstractFrameContentManager::setDocument(document);

    if (_document) {
        connect(_document, &Document::actionPointChanged,
                this, &ActionPointManager::onItemChanged);
        connect(_document, &Document::actionPointAdded,
                this, &ActionPointManager::onItemAdded);
        connect(_document, &Document::actionPointAboutToBeRemoved,
                this, &ActionPointManager::onItemAboutToBeRemoved);
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
        return QPoint(ap.location.x, ap.location.y);

    case PropertyId::PARAMETER:
        return (int)ap.parameter;
    };

    return QVariant();
}

bool ActionPointManager::setData(int index, int id, const QVariant& value)
{
    if (_frame == nullptr
        || index < 0 || (unsigned)index >= _frame->actionPoints.size()) {

        return false;
    }

    const MS::ActionPoint& oldAp = _frame->actionPoints.at(index);
    MS::ActionPoint ap = oldAp;

    switch ((PropertyId)id) {
    case PropertyId::LOCATION:
        ap.location.x = value.toPoint().x();
        ap.location.y = value.toPoint().y();
        break;

    case PropertyId::PARAMETER:
        ap.parameter = value.toUInt();
        break;
    };

    if (ap != oldAp) {
        _document->undoStack()->push(
            new ChangeActionPoint(_document, _frame, index, ap));

        return true;
    }
    return false;
}

SelectedItem::Type ActionPointManager::itemType() const
{
    return SelectedItem::ACTION_POINT;
}

EntityHitboxManager::EntityHitboxManager(QObject* parent)
    : AbstractFrameContentManager(parent)
{
    using EHT = UnTech::MetaSprite::EntityHitboxType;
    using Type = PropertyType;

    setTitle(tr("Entity Hitbox"));

    addProperty(tr("AABB"), PropertyId::AABB, Type::RECT,
                QRect(int_ms8_t::MIN, int_ms8_t::MIN, UINT8_MAX, UINT8_MAX));
    addProperty(tr("Hitbox Type"), PropertyId::HITBOX_TYPE, Type::COMBO,
                enumComboNames(EHT::enumMap), enumComboDataList(EHT::enumMap));
}

void EntityHitboxManager::setDocument(Document* document)
{
    AbstractFrameContentManager::setDocument(document);

    if (_document) {
        connect(_document, &Document::entityHitboxChanged,
                this, &EntityHitboxManager::onItemChanged);
        connect(_document, &Document::entityHitboxAdded,
                this, &EntityHitboxManager::onItemAdded);
        connect(_document, &Document::entityHitboxAboutToBeRemoved,
                this, &EntityHitboxManager::onItemAboutToBeRemoved);
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
        return QRect(eh.aabb.x, eh.aabb.y,
                     eh.aabb.width, eh.aabb.height);

    case PropertyId::HITBOX_TYPE:
        return int(eh.hitboxType.value());
    };

    return QVariant();
}

bool EntityHitboxManager::setData(int index, int id, const QVariant& value)
{
    using EntityHitboxType = UnTech::MetaSprite::EntityHitboxType;

    if (_frame == nullptr
        || index < 0 || (unsigned)index >= _frame->entityHitboxes.size()) {

        return false;
    }

    const MS::EntityHitbox& oldeh = _frame->entityHitboxes.at(index);
    MS::EntityHitbox eh = oldeh;

    switch ((PropertyId)id) {
    case PropertyId::AABB:
        eh.aabb.x = value.toRect().x();
        eh.aabb.y = value.toRect().y();
        eh.aabb.width = value.toRect().width();
        eh.aabb.height = value.toRect().height();
        break;

    case PropertyId::HITBOX_TYPE:
        eh.hitboxType = (EntityHitboxType::Enum)value.toInt();
        break;
    };

    if (eh != oldeh) {
        _document->undoStack()->push(
            new ChangeEntityHitbox(_document, _frame, index, eh));

        return true;
    }
    return false;
}

SelectedItem::Type EntityHitboxManager::itemType() const
{
    return SelectedItem::ENTITY_HITBOX;
}