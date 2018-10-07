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

using namespace UnTech::GuiQt::MetaSprite;
using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

const QStringList FrameObjectManager::SIZE_STRINGS({ QString::fromUtf8("Small"),
                                                     QString::fromUtf8("Large") });

AbstractFrameContentManager::AbstractFrameContentManager(QObject* parent)
    : PropertyTableManager(parent)
    , _document(nullptr)
    , _frame(nullptr)
{
}

void AbstractFrameContentManager::setDocument(Document* document)
{
    if (_document) {
        _document->frameMap()->disconnect(this);
    }
    _document = document;

    _frame = nullptr;
    emit dataReset();

    if (_document) {
        onSelectedFrameChanged();

        connect(_document->frameMap(), &FrameMap::selectedItemChanged,
                this, &AbstractFrameContentManager::onSelectedFrameChanged);

        connect(_document->frameMap(), &FrameMap::frameLocationChanged,
                this, &AbstractFrameContentManager::onFrameLocationChanged);
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
    const SI::Frame* frame = _document->frameMap()->selectedFrame();

    if (_frame != frame) {
        _frame = frame;
        dataReset();
    }
}

void AbstractFrameContentManager::onFrameLocationChanged(const void* frame)
{
    if (frame == _frame) {
        emit dataChanged();
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

    const SI::FrameObject& obj = _frame->objects.at(index);

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
    if (_frame == nullptr
        || index < 0 || (unsigned)index >= _frame->objects.size()) {

        return;
    }

    const SI::FrameObject& obj = _frame->objects.at(index);

    switch ((PropertyId)id) {
    case PropertyId::LOCATION: {
        const usize s = _frame->location.aabb.size();
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

    if (_frame == nullptr) {
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

    const SI::ActionPoint& ap = _frame->actionPoints.at(index);

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
    if (_frame == nullptr
        || index < 0 || (unsigned)index >= _frame->actionPoints.size()) {

        return;
    }

    switch ((PropertyId)id) {
    case PropertyId::LOCATION: {
        const usize s = _frame->location.aabb.size();

        param1 = QPoint(0, 0);
        param2 = QPoint(s.width - 1, s.height - 1);
    } break;

    case PropertyId::PARAMETER:
        break;
    };
}

bool ActionPointManager::setData(int index, int id, const QVariant& value)
{
    if (_frame == nullptr) {
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

    const SI::EntityHitbox& eh = _frame->entityHitboxes.at(index);

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
    if (_frame == nullptr
        || index < 0 || (unsigned)index >= _frame->entityHitboxes.size()) {

        return;
    }

    Q_UNUSED(param2);

    switch ((PropertyId)id) {
    case PropertyId::AABB: {
        const usize s = _frame->location.aabb.size();
        param1 = QRect(0, 0, s.width, s.height);
    } break;

    case PropertyId::HITBOX_TYPE:
        break;
    };
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
            index, toUrect(value.toRect()));

    case PropertyId::HITBOX_TYPE:
        return _document->entityHitboxList()->editSelectedList_setEntityHitboxType(
            index, EntityHitboxType::from_romValue(value.toInt()));
    };

    return false;
}
