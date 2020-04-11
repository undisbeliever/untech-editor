/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/metatiles/mtgraphicsscenes.h"
#include "models/rooms/rooms.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {

namespace Entity::EntityRomEntries {
class ResourceItem;
}

namespace Rooms {
class ResourceItem;
class EntityGroupGraphicsItem;

namespace RM = UnTech::Rooms;

class RoomGraphicsScene final : public MetaTiles::MtGraphicsScene {
    Q_OBJECT

public:
    RoomGraphicsScene(MetaTiles::Style* style, MetaTiles::MtTileset::MtTilesetRenderer* renderer, QObject* parent);
    ~RoomGraphicsScene() = default;

    void setResourceItem(ResourceItem* room);

    virtual const grid_t& grid() const final;
    virtual const upoint_vectorset& gridSelection() const final;

protected:
    virtual void setGridSelection(upoint_vectorset&& selectedCells) final;

    virtual void tilesetItemChanged(MetaTiles::MtTileset::ResourceItem* newTileset, MetaTiles::MtTileset::ResourceItem* oldTileset) final;

private:
    ResourceItem* _room;
};

class EditableRoomGraphicsScene final : public MetaTiles::MtEditableGraphicsScene {
    Q_OBJECT

public:
    EditableRoomGraphicsScene(MetaTiles::Style* style, MetaTiles::MtTileset::MtTilesetRenderer* renderer, QObject* parent);
    ~EditableRoomGraphicsScene() = default;

    void setResourceItem(ResourceItem* room);

    virtual const grid_t& grid() const final;
    virtual const upoint_vectorset& gridSelection() const final;

    virtual void placeTiles(const selection_grid_t& tiles, point location, const QString& text, bool firstClick) final;

protected:
    virtual void setGridSelection(upoint_vectorset&& selectedCells) final;

    virtual void tilesetItemChanged(MetaTiles::MtTileset::ResourceItem* newTileset, MetaTiles::MtTileset::ResourceItem* oldTileset) final;

    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
    virtual void dragMoveEvent(QGraphicsSceneDragDropEvent* event) override;
    virtual void dropEvent(QGraphicsSceneDragDropEvent* event) override;

private slots:
    void updateValidEntityArea();
    void onSceneSelectionChanged();

    void onEntityGroupAccessorSelectionChanged();
    void onEntityEntriesAccessorSelectionChanged();

    void updateAllEntities();
    void onEntityEntriesListChanged(size_t groupIndex);
    void onEntityEntriesDataChanged(size_t groupIndex, size_t childIndex);

private:
    optional<const RM::RoomInput&> roomInput() const;

    void commitMovedItems();

private:
    ResourceItem* _room;

    QRect _validEntityArea;

    QList<EntityGroupGraphicsItem*> _entityGroups;
    const Entity::EntityRomEntries::ResourceItem* _entitiesResourceItem;

    bool _groupSelected;

    bool _inOnAccessorSelectionChanged;
    bool _inOnSceneSelectionChanged;
};

}
}
}
