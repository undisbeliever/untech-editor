/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "managers.h"
#include "accessors.h"
#include "resourceitem.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/project.h"
#include "models/project/project.h"

using namespace UnTech::GuiQt::Rooms;

const QStringList RoomEntranceManager::ENTRANCE_DIRECTION_STRINGS{
    QStringLiteral("Down Right"),
    QStringLiteral("Down Left"),
    QStringLiteral("Up Right"),
    QStringLiteral("Up Left"),
};

RoomPropertyManager::RoomPropertyManager(QObject* parent)
    : PropertyListManager(parent)
    , _resourceItem(nullptr)
{
    using Type = UnTech::GuiQt::PropertyType;
    using RoomInput = UnTech::Rooms::RoomInput;

    const QSize MIN_MAP_SIZE{ RoomInput::MIN_MAP_WIDTH, RoomInput::MIN_MAP_HEIGHT };
    const QSize MAX_MAP_SIZE = fromUsize(MapGrid::maxSize());

    addProperty(tr("Name"), NAME, Type::IDSTRING);
    addProperty(tr("Scene"), SCENE, Type::STRING_COMBO);
    addProperty(tr("Map Size"), MAP_SIZE, Type::SIZE, MIN_MAP_SIZE, MAX_MAP_SIZE);
}

void RoomPropertyManager::setResourceItem(ResourceItem* item)
{
    if (_resourceItem == item) {
        return;
    }

    if (_resourceItem) {
        _resourceItem->disconnect(this);
        _resourceItem->mapGrid()->disconnect(this);
    }
    _resourceItem = item;

    setEnabled(_resourceItem != nullptr);

    if (_resourceItem) {
        connect(_resourceItem, &ResourceItem::nameChanged,
                this, &RoomPropertyManager::dataChanged);
        connect(_resourceItem, &ResourceItem::sceneChanged,
                this, &RoomPropertyManager::dataChanged);
        connect(_resourceItem->mapGrid(), &MapGrid::gridResized,
                this, &RoomPropertyManager::dataChanged);
        connect(_resourceItem, &ResourceItem::resourceComplied,
                this, &RoomPropertyManager::dataChanged);
    }

    emit dataChanged();
}

void RoomPropertyManager::updateParameters(int id, QVariant& param1, QVariant& param2) const
{
    Q_UNUSED(param2);

    if (_resourceItem == nullptr) {
        return;
    }

    if (id == SCENE) {
        auto* project = _resourceItem->project()->projectFile();
        Q_ASSERT(project);

        param1 = convertNameList(project->resourceScenes.scenes);
    }
}

QVariant RoomPropertyManager::data(int id) const
{
    if (_resourceItem == nullptr) {
        return QVariant();
    }

    const auto bi = _resourceItem->roomInput();
    if (!bi) {
        return QVariant();
    }

    switch ((PropertyId)id) {
    case NAME:
        return QString::fromStdString(bi->name);

    case SCENE:
        return QString::fromStdString(bi->scene);

    case MAP_SIZE:
        return fromUsize(bi->map.size());
    }

    return QVariant();
}

bool RoomPropertyManager::setData(int id, const QVariant& value)
{
    Q_ASSERT(_resourceItem);

    switch ((PropertyId)id) {
    case NAME:
        return _resourceItem->edit_setName(value.toString().toStdString());

    case SCENE:
        return _resourceItem->edit_setScene(value.toString().toStdString());

    case MAP_SIZE:
        return _resourceItem->mapGrid()->editGrid_resizeGrid(toUsize(value.toSize()));
    }

    return false;
}

RoomEntranceManager::RoomEntranceManager(QObject* parent)
    : Accessor::ListAccessorTableManager(parent)
{
    addProperty(tr("Name"), NAME, PropertyType::IDSTRING);
    addProperty(tr("Position"), POSITION, PropertyType::POINT);
    addProperty(tr("Orientation"), ORIENTATION, PropertyType::COMBO, ENTRANCE_DIRECTION_STRINGS, qVariantRange(ENTRANCE_DIRECTION_STRINGS.size()));
}

void RoomEntranceManager::setResourceItem(ResourceItem* item)
{
    setAccessor(item ? item->roomEntrances() : nullptr);
}

inline RoomEntranceList* RoomEntranceManager::accessor() const
{
    return static_cast<RoomEntranceList*>(ListAccessorTableManager::accessor());
}

void RoomEntranceManager::updateParameters(int, int id, QVariant& param1, QVariant& param2) const
{
    const auto* reList = accessor();
    if (reList == nullptr) {
        return;
    }

    const auto ri = reList->resourceItem()->roomInput();
    if (!ri) {
        return;
    }

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
    case PropertyId::ORIENTATION:
        break;

    case PropertyId::POSITION:
        param1 = QPoint(0, 0);
        param2 = QPoint(ri->mapRight(), ri->mapBottom());
        break;
    }
}

QVariant RoomEntranceManager::data(int index, int id) const
{
    const auto* reList = accessor();
    if (reList == nullptr
        || index < 0
        || (unsigned)index >= reList->size()) {

        return QVariant();
    }

    const RM::RoomEntrance& en = reList->list()->at(index);

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
        return QString::fromStdString(en.name);

    case PropertyId::POSITION:
        return fromUpoint(en.position);

    case PropertyId::ORIENTATION:
        return static_cast<int>(en.orientation);
    }

    return QVariant();
}

bool RoomEntranceManager::setData(int index, int id, const QVariant& value)
{
    auto* reList = accessor();
    if (reList == nullptr
        || index < 0
        || (unsigned)index >= reList->size()) {

        return false;
    }

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
        return reList->edit_setName(index, value.toString().toStdString());

    case PropertyId::POSITION:
        return reList->edit_setPosition(index, toUpoint(value.toPoint()));

    case PropertyId::ORIENTATION:
        return reList->edit_setOrientation(index, static_cast<RM::RoomEntranceOrientation>(value.toInt()));
    }

    return false;
}
