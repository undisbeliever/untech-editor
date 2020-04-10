/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "roomgraphicsscenes.h"
#include "accessors.h"
#include "entitygraphicsitem.h"
#include "resourceitem.h"
#include "gui-qt/project.h"
#include "gui-qt/staticresourcelist.h"

#include <QGraphicsSceneMouseEvent>

using namespace UnTech;
using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Rooms;

RoomGraphicsScene::RoomGraphicsScene(MetaTiles::Style* style, MetaTiles::MtTileset::MtTilesetRenderer* renderer, QObject* parent)
    : MtGraphicsScene(style, renderer, parent)
    , _room(nullptr)
{
}

void RoomGraphicsScene::setResourceItem(ResourceItem* room)
{
    if (_room == room) {
        return;
    }

    if (_room) {
        _room->mapGrid()->disconnect(this);
    }
    _room = room;

    emit gridResized();
    emit gridSelectionChanged();

    if (room) {
        connect(room->mapGrid(), &MapGrid::gridChanged,
                this, &RoomGraphicsScene::gridChanged);

        connect(room->mapGrid(), &MapGrid::gridReset,
                this, &RoomGraphicsScene::gridResized);

        connect(room->mapGrid(), &MapGrid::gridResized,
                this, &RoomGraphicsScene::gridResized);

        connect(room->mapGrid(), &MapGrid::selectedCellsChanged,
                this, &RoomGraphicsScene::gridSelectionChanged);
    }
}

const MetaTiles::MtGraphicsScene::grid_t& RoomGraphicsScene::grid() const
{
    if (_room) {
        if (auto ri = _room->roomInput()) {
            return ri->map;
        }
    }

    return BLANK_GRID;
}

const upoint_vectorset& RoomGraphicsScene::gridSelection() const
{
    if (_room) {
        return _room->mapGrid()->selectedCells();
    }

    return BLANK_GRID_SELECTION;
}

void RoomGraphicsScene::setGridSelection(upoint_vectorset&& selectedCells)
{
    if (_room) {
        return _room->mapGrid()->setSelectedCells(std::move(selectedCells));
    }
}

void RoomGraphicsScene::tilesetItemChanged(MetaTiles::MtTileset::ResourceItem* newTileset, MetaTiles::MtTileset::ResourceItem* oldTileset)
{
    Q_UNUSED(newTileset);
    Q_UNUSED(oldTileset);
}

EditableRoomGraphicsScene::EditableRoomGraphicsScene(MetaTiles::Style* style, MetaTiles::MtTileset::MtTilesetRenderer* renderer, QObject* parent)
    : MtEditableGraphicsScene(style, renderer, parent)
    , _room(nullptr)
    , _entitiesResourceItem(nullptr)
    , _groupSelected(false)
    , _inOnAccessorSelectionChanged(false)
    , _inOnSceneSelectionChanged(false)
{
    connect(this, &EditableRoomGraphicsScene::selectionChanged,
            this, &EditableRoomGraphicsScene::onSceneSelectionChanged);
}

void EditableRoomGraphicsScene::setResourceItem(ResourceItem* room)
{
    if (_room == room) {
        return;
    }

    if (_room) {
        _room->mapGrid()->disconnect(this);
        _room->entityGroups()->disconnect(this);
        _room->entityEntries()->disconnect(this);
    }
    _room = room;

    if (_entitiesResourceItem) {
        _entitiesResourceItem->disconnect(this);
    }
    _entitiesResourceItem = room ? room->project()->staticResources()->entities() : nullptr;

    emit gridResized();
    emit gridSelectionChanged();

    updateAllEntities();

    if (room) {
        onEntityEntriesAccessorSelectionChanged();
        onSceneSelectionChanged();
        onEntityGroupAccessorSelectionChanged();

        connect(room->mapGrid(), &MapGrid::gridChanged,
                this, &EditableRoomGraphicsScene::gridChanged);

        connect(room->mapGrid(), &MapGrid::gridReset,
                this, &EditableRoomGraphicsScene::gridResized);

        connect(room->mapGrid(), &MapGrid::gridResized,
                this, &EditableRoomGraphicsScene::gridResized);

        connect(room->mapGrid(), &MapGrid::selectedCellsChanged,
                this, &EditableRoomGraphicsScene::gridSelectionChanged);

        connect(room->entityGroups(), &EntityGroupList::listReset,
                this, &EditableRoomGraphicsScene::updateAllEntities);
        connect(room->entityGroups(), &EntityGroupList::listChanged,
                this, &EditableRoomGraphicsScene::updateAllEntities);

        connect(room->entityGroups(), &EntityGroupList::selectedIndexChanged,
                this, &EditableRoomGraphicsScene::onEntityGroupAccessorSelectionChanged);
        connect(room->entityEntries(), &EntityEntriesList::selectedIndexesChanged,
                this, &EditableRoomGraphicsScene::onEntityEntriesAccessorSelectionChanged);

        connect(room->entityEntries(), &EntityEntriesList::listReset,
                this, &EditableRoomGraphicsScene::updateAllEntities);
        connect(room->entityEntries(), &EntityEntriesList::listChanged,
                this, &EditableRoomGraphicsScene::onEntityEntriesListChanged);
        connect(room->entityEntries(), &EntityEntriesList::dataChanged,
                this, &EditableRoomGraphicsScene::onEntityEntriesDataChanged);

        connect(_entitiesResourceItem, &Entity::EntityRomEntries::ResourceItem::entityPixmapsChanged,
                this, &EditableRoomGraphicsScene::updateAllEntities);
    }
}

optional<const RM::RoomInput&> EditableRoomGraphicsScene::roomInput() const
{
    return _room ? _room->roomInput() : optional<const RM::RoomInput&>{};
}

const MetaTiles::MtGraphicsScene::grid_t& EditableRoomGraphicsScene::grid() const
{
    if (_room) {
        if (auto ri = _room->roomInput()) {
            return ri->map;
        }
    }

    return BLANK_GRID;
}

const upoint_vectorset& EditableRoomGraphicsScene::gridSelection() const
{
    if (_room) {
        return _room->mapGrid()->selectedCells();
    }

    return BLANK_GRID_SELECTION;
}

void EditableRoomGraphicsScene::setGridSelection(upoint_vectorset&& selectedCells)
{
    if (_room) {
        return _room->mapGrid()->setSelectedCells(std::move(selectedCells));
    }
}

void EditableRoomGraphicsScene::placeTiles(const selection_grid_t& tiles, point location, const QString& text, bool firstClick)
{
    if (_room) {
        _room->mapGrid()->editGrid_placeTiles(location, tiles, text, firstClick);
    }
}

void EditableRoomGraphicsScene::tilesetItemChanged(MetaTiles::MtTileset::ResourceItem* newTileset, MetaTiles::MtTileset::ResourceItem* oldTileset)
{
    Q_UNUSED(newTileset);
    Q_UNUSED(oldTileset);
}

void EditableRoomGraphicsScene::onSceneSelectionChanged()
{
    if (_room == nullptr || _inOnAccessorSelectionChanged) {
        return;
    }

    Q_ASSERT(_inOnSceneSelectionChanged == false);
    _inOnSceneSelectionChanged = true;

    std::vector<std::pair<size_t, size_t>> sel;

    for (int groupIndex = 0; groupIndex < _entityGroups.size(); groupIndex++) {
        auto* groupItem = _entityGroups.at(groupIndex);

        for (int childIndex = 0; childIndex < groupItem->nEntities(); childIndex++) {
            auto* item = groupItem->at(childIndex);
            if (item->isSelected()) {
                sel.emplace_back(groupIndex, childIndex);
            }
        }
    }

    const bool selEmpty = sel.empty();

    _room->entityEntries()->setSelectedIndexes(std::move(sel));

    if (selEmpty) {
        _room->entityGroups()->unselectItem();
    }

    _inOnSceneSelectionChanged = false;

    // unhighlight group (if necessary)
    onEntityGroupAccessorSelectionChanged();
}

void EditableRoomGraphicsScene::onEntityGroupAccessorSelectionChanged()
{
    if (_room == nullptr || _inOnSceneSelectionChanged) {
        return;
    }

    Q_ASSERT(_inOnAccessorSelectionChanged == false);
    _inOnAccessorSelectionChanged = true;

    constexpr qreal NORMAL_OPACITY = 1.0f;
    constexpr qreal SELECTED_OPACITY = 1.0f;
    constexpr qreal NOT_SELECTED_OPACITY = 0.25f;

    const int sel = _room->entityGroups()->selectedIndex();

    const bool onlyGroupSelected = _room->entityEntries()->selectedIndexes().empty()
                                   && sel >= 0 && sel < _entityGroups.size();

    if (onlyGroupSelected) {
        for (int i = 0; i < _entityGroups.size(); i++) {
            _entityGroups.at(i)->setOpacity(i == sel ? SELECTED_OPACITY : NOT_SELECTED_OPACITY);
        }
        _groupSelected = true;
    }
    else {
        if (_groupSelected) {
            for (auto* item : _entityGroups) {
                item->setOpacity(NORMAL_OPACITY);
            }
            _groupSelected = false;
        }
    }

    _inOnAccessorSelectionChanged = false;
}

void EditableRoomGraphicsScene::onEntityEntriesAccessorSelectionChanged()
{
    if (_room == nullptr || _inOnSceneSelectionChanged) {
        return;
    }

    Q_ASSERT(_inOnAccessorSelectionChanged == false);
    _inOnAccessorSelectionChanged = true;

    // ::SHOULDDO optimize to an O(n) algorithm::

    Q_ASSERT(_room);

    const auto& sel = _room->entityEntries()->selectedIndexes();

    for (int groupIndex = 0; groupIndex < _entityGroups.size(); groupIndex++) {
        auto* groupItem = _entityGroups.at(groupIndex);

        for (int childIndex = 0; childIndex < groupItem->nEntities(); childIndex++) {
            groupItem->at(childIndex)->setSelected(sel.contains({ groupIndex, childIndex }));
        }
    }

    _inOnAccessorSelectionChanged = false;

    onEntityGroupAccessorSelectionChanged();
}

void EditableRoomGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    MtEditableGraphicsScene::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton) {
        commitMovedItems();
    }
}

void EditableRoomGraphicsScene::updateAllEntities()
{
    const auto ri = roomInput();
    const int groupSize = ri ? ri->entityGroups.size() : 0;

    while (_entityGroups.size() > groupSize) {
        delete _entityGroups.takeLast();
    }
    for (int i = _entityGroups.size(); i < groupSize; i++) {
        auto* groupItem = new EntityGroupGraphicsItem(i);
        _entityGroups.append(groupItem);
        addItem(groupItem);
    }

    if (groupSize == 0) {
        return;
    }

    Q_ASSERT(_entitiesResourceItem);

    // Using for each on _entities as _entities.at(int) returns a constant reference
    for (int i = 0; i < _entityGroups.size(); i++) {
        _entityGroups.at(i)->updateAllEntities(ri->entityGroups.at(i).entities, *_entitiesResourceItem);
    }
}

void EditableRoomGraphicsScene::onEntityEntriesListChanged(size_t groupIndex)
{
    const auto ri = roomInput();
    if (!ri) {
        return;
    }

    Q_ASSERT(_entitiesResourceItem);

    _entityGroups.at(groupIndex)->updateAllEntities(ri->entityGroups.at(groupIndex).entities, *_entitiesResourceItem);
}

void EditableRoomGraphicsScene::onEntityEntriesDataChanged(size_t groupIndex, size_t childIndex)
{
    const auto ri = roomInput();
    if (!ri) {
        return;
    }

    Q_ASSERT(_entitiesResourceItem);

    auto* item = _entityGroups.at(groupIndex)->at(childIndex);

    item->updateEntity(ri->entityGroups.at(groupIndex).entities.at(childIndex),
                       *_entitiesResourceItem);
}
