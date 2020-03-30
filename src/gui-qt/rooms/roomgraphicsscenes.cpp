/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "roomgraphicsscenes.h"
#include "accessors.h"
#include "resourceitem.h"

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
{
}

void EditableRoomGraphicsScene::setResourceItem(ResourceItem* room)
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
                this, &EditableRoomGraphicsScene::gridChanged);

        connect(room->mapGrid(), &MapGrid::gridReset,
                this, &EditableRoomGraphicsScene::gridResized);

        connect(room->mapGrid(), &MapGrid::gridResized,
                this, &EditableRoomGraphicsScene::gridResized);

        connect(room->mapGrid(), &MapGrid::selectedCellsChanged,
                this, &EditableRoomGraphicsScene::gridSelectionChanged);
    }
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
