/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "entitygraphicsitem.h"
#include "roomgraphicsscenes.h"
#include "gui-qt/accessor/abstractaccessors.hpp"
#include "gui-qt/accessor/gridundohelper.h"
#include "gui-qt/accessor/nestedlistaccessors.hpp"
#include "gui-qt/project.h"

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::Rooms;

MapGrid::MapGrid(ResourceItem* room)
    : AbstractGridAccessor(room)
{
    Q_ASSERT(room);

    connect(room, &ResourceItem::resourceLoaded,
            this, &MapGrid::gridReset);
}

UnTech::usize MapGrid::size() const
{
    if (auto* data = resourceItem()->data()) {
        return data->map.size();
    }
    return usize(0, 0);
}

bool MapGrid::editGrid_resizeGrid(const usize& size)
{
    return UndoHelper(this).resizeGrid(
        size, 0,
        tr("Resize map"));
}

bool MapGrid::editGrid_placeTiles(const point& location, const grid<uint16_t>& tiles, const QString& text, bool firstClick)
{
    constexpr unsigned MAX = std::numeric_limits<DataT>::max();

    return UndoHelper(this).editCellsMergeWithCroppingAndCellTest(
        location, tiles, text, firstClick,
        [&](const uint16_t& t) -> optional<uint8_t> { return t < MAX ? t : optional<uint8_t>{}; });
}

template <>
const NamedList<RM::RoomEntrance>* NamedListAccessor<RM::RoomEntrance, ResourceItem>::list() const
{
    if (auto ri = resourceItem()->roomInput()) {
        return &ri->entrances;
    }
    else {
        return nullptr;
    }
}

template <>
NamedList<RM::RoomEntrance>* NamedListAccessor<RM::RoomEntrance, ResourceItem>::getList()
{
    if (auto ri = resourceItem()->dataEditable()) {
        return &ri->entrances;
    }
    else {
        return nullptr;
    }
}

RoomEntranceList::RoomEntranceList(ResourceItem* resourceItem)
    : NamedListAccessor(resourceItem, RM::MAX_ROOM_ENTRANCES)
{
}

QString RoomEntranceList::typeName() const
{
    return tr("Room Entrance");
}

QString RoomEntranceList::typeNamePlural() const
{
    return tr("Room Entrances");
}

bool RoomEntranceList::edit_setPosition(index_type index, const upoint& position)
{
    return UndoHelper(this).editField(
        index, position,
        tr("Edit Room Entrance Position"),
        [](RM::RoomEntrance& e) -> upoint& { return e.position; });
}

bool RoomEntranceList::edit_setOrientation(index_type index, RM::RoomEntranceOrientation orientation)
{
    return UndoHelper(this).editField(
        index, orientation,
        tr("Edit Room Entrance Orientation"),
        [](RM::RoomEntrance& e) -> RM::RoomEntranceOrientation& { return e.orientation; });
}

template <>
const NamedList<RM::EntityGroup>* NamedListAccessor<RM::EntityGroup, ResourceItem>::list() const
{
    if (auto ri = resourceItem()->roomInput()) {
        return &ri->entityGroups;
    }
    else {
        return nullptr;
    }
}

template <>
NamedList<RM::EntityGroup>* NamedListAccessor<RM::EntityGroup, ResourceItem>::getList()
{
    if (auto ri = resourceItem()->dataEditable()) {
        return &ri->entityGroups;
    }
    else {
        return nullptr;
    }
}

EntityGroupList::EntityGroupList(ResourceItem* resourceItem)
    : NamedListAccessor(resourceItem, RM::MAX_ENTITY_GROUPS)
{
}

QString EntityGroupList::typeName() const
{
    return tr("Entity Group");
}

QString EntityGroupList::typeNamePlural() const
{
    return tr("Entity Groups");
}

template <>
const NamedList<RM::EntityGroup>* NestedNlvMulitpleSelectionAccessor<RM::EntityGroup, RM::EntityEntry, ResourceItem>::parentList() const
{
    if (auto ri = resourceItem()->data()) {
        return &ri->entityGroups;
    }
    else {
        return nullptr;
    }
}

template <>
const std::vector<RM::EntityEntry>& NestedNlvMulitpleSelectionAccessor<RM::EntityGroup, RM::EntityEntry, ResourceItem>::childList(const RM::EntityGroup& eg)
{
    return eg.entities;
}

template <>
NamedList<RM::EntityGroup>* NestedNlvMulitpleSelectionAccessor<RM::EntityGroup, RM::EntityEntry, ResourceItem>::getParentList()
{
    if (auto ri = resourceItem()->dataEditable()) {
        return &ri->entityGroups;
    }
    else {
        return nullptr;
    }
}

template <>
std::vector<RM::EntityEntry>& NestedNlvMulitpleSelectionAccessor<RM::EntityGroup, RM::EntityEntry, ResourceItem>::getChildList(RM::EntityGroup& eg)
{
    return eg.entities;
}

EntityEntriesList::EntityEntriesList(ResourceItem* item)
    : NestedNlvMulitpleSelectionAccessor(item, RM::MAX_ENTITY_ENTRIES)
{
    connect(this, &EntityEntriesList::selectedIndexesChanged,
            this, &EntityEntriesList::onSelectedIndexesChanged);

    connect(item->entityGroups(), &EntityGroupList::listChanged,
            this, &EntityEntriesList::clearSelection);
}

void EntityEntriesList::onSelectedIndexesChanged()
{
    auto& sel = selectedIndexes();
    if (!sel.empty()) {
        if (auto pi = parentIndex()) {
            resourceItem()->entityGroups()->setSelectedIndex(*pi);
        }
        else {
            resourceItem()->entityGroups()->unselectItem();
        }
    }
}

QString EntityEntriesList::typeName() const
{
    return tr("Room Entity");
}

QString EntityEntriesList::typeNamePlural() const
{
    return tr("Room Entities");
}

bool EntityEntriesList::addEntity(NestedNlvMulitpleSelectionAccessor::index_type groupIndex, const idstring& entityId, const point& position)
{
    return UndoHelper(this).addItem(
        groupIndex, childListSize(groupIndex),
        RM::EntityEntry{ .name = {},
                         .entityId = entityId,
                         .position = position },
        tr("Add Room Entity"));
}

bool EntityEntriesList::edit_setName(index_type groupIndex, index_type entryIndex, const idstring& name)
{
    return UndoHelper(this).editField(
        groupIndex, entryIndex,
        name,
        tr("Edit Entity Name"),
        [](RM::EntityEntry& e) -> idstring& { return e.name; });
}

bool EntityEntriesList::edit_setEntityId(index_type groupIndex, index_type entryIndex, const idstring& entityId)
{
    return UndoHelper(this).editField(
        groupIndex, entryIndex,
        entityId,
        tr("Edit Entity Id"),
        [](RM::EntityEntry& e) -> idstring& { return e.entityId; });
}

bool EntityEntriesList::edit_setPosition(index_type groupIndex, index_type entryIndex, const point& position)
{
    return UndoHelper(this).editField(
        groupIndex, entryIndex,
        position,
        tr("Edit Entity Position"),
        [](RM::EntityEntry& e) -> point& { return e.position; });
}

bool EntityEntriesList::edit_setParameter(index_type groupIndex, index_type entryIndex, const std::string& parameter)
{
    return UndoHelper(this).editField(
        groupIndex, entryIndex,
        parameter,
        tr("Edit Entity Parameter"),
        [](RM::EntityEntry& e) -> std::string& { return e.parameter; });
}

void EditableRoomGraphicsScene::commitMovedItems()
{
    if (_room == nullptr) {
        return;
    }

    if (_room->entityEntries()->selectedIndexes().empty() == false) {
        EntityEntriesList::UndoHelper(_room->entityEntries())
            .editSelectedItems(
                tr("Move Room Entities"),
                [this](RM::EntityEntry& ee, size_t groupIndex, size_t childIndex) {
                    const auto pos = _entityGroups.at(groupIndex)->at(childIndex)->pos();
                    ee.position.x = pos.x();
                    ee.position.y = pos.y();
                });
    }
}

using namespace UnTech::GuiQt;
template class Accessor::NamedListAccessor<RM::EntityGroup, ResourceItem>;
template class Accessor::NamedListAccessor<RM::RoomEntrance, ResourceItem>;
template class Accessor::NestedNlvMulitpleSelectionAccessor<RM::EntityGroup, RM::EntityEntry, ResourceItem>;
