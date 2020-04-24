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

RoomPropertyManager::RoomPropertyManager(QObject* parent)
    : PropertyListManager(parent)
    , _resourceItem(nullptr)
{
    using Type = UnTech::GuiQt::PropertyType;
    using RoomInput = UnTech::Rooms::RoomInput;

    constexpr QSize MIN_MAP_SIZE{ RoomInput::MIN_MAP_WIDTH, RoomInput::MIN_MAP_HEIGHT };
    constexpr QSize MAX_MAP_SIZE{ RoomInput::MAX_MAP_WIDTH, RoomInput::MAX_MAP_HEIGHT };

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
