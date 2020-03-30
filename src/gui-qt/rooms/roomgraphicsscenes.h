/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/metatiles/mtgraphicsscenes.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace Rooms {
class ResourceItem;

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

    void tilesetItemChanged(MetaTiles::MtTileset::ResourceItem* newTileset, MetaTiles::MtTileset::ResourceItem* oldTileset) final;

private:
    ResourceItem* _room;
};

}
}
}
