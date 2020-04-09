/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "resourceitem.h"
#include "gui-qt/accessor/abstractaccessors.h"
#include "gui-qt/accessor/nestedlistaccessors.h"
#include "models/common/grid.h"
#include <QObject>
#include <cstdint>
#include <tuple>

namespace UnTech {
namespace GuiQt {
namespace Rooms {

namespace RM = UnTech::Rooms;

class MapGrid final : public Accessor::AbstractGridAccessor {
    Q_OBJECT

public:
    using DataT = uint8_t;
    using GridT = UnTech::grid<uint8_t>;
    using ArgsT = std::tuple<>;

    using UndoHelper = Accessor::GridUndoHelper<MapGrid>;

    static UnTech::usize maxSize() { return usize(255, 128); }

public:
    MapGrid(ResourceItem* room);
    ~MapGrid() = default;

    ResourceItem* resourceItem() const { return static_cast<ResourceItem*>(AbstractGridAccessor::resourceItem()); }

    virtual usize size() const final;

    bool editGrid_resizeGrid(const usize& size);
    bool editGrid_placeTiles(const point& location, const grid<uint16_t>& tiles, const QString& text, bool firstClick);

protected:
    friend class Accessor::GridUndoHelper<MapGrid>;
    GridT* getGrid()
    {
        if (auto* data = resourceItem()->dataEditable()) {
            return &data->map;
        }
        return nullptr;
    }

    ArgsT selectedGridTuple() const { return std::make_tuple(); }
};

class EntityGroupList final : public Accessor::NamedListAccessor<RM::EntityGroup, ResourceItem> {
    Q_OBJECT

public:
    using UndoHelper = Accessor::ListAndSelectionUndoHelper<EntityGroupList>;

public:
    EntityGroupList(ResourceItem* resourceItem);
    ~EntityGroupList() = default;

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;
};

class EntityEntriesList final : public Accessor::NestedNlvMulitpleSelectionAccessor<RM::EntityGroup, RM::EntityEntry, ResourceItem> {
    Q_OBJECT

    using UndoHelper = Accessor::NestedListAndMultipleSelectionUndoHelper<EntityEntriesList>;

public:
    EntityEntriesList(ResourceItem* item);

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

    bool edit_setName(index_type groupIndex, index_type entryIndex, const idstring& name);
    bool edit_setEntityId(index_type groupIndex, index_type entryIndex, const idstring& entityId);
    bool edit_setPosition(index_type groupIndex, index_type entryIndex, const point& position);

private slots:
    void onSelectedIndexesChanged();
};

}
}
}
