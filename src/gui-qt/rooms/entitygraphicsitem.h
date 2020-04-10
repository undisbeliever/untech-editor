/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/graphics/aabbselectionoutlineitem.h"
#include "gui-qt/entity/entity-rom-entries/resourceitem.h"
#include "models/common/aabb.h"
#include "models/rooms/rooms.h"
#include <QGraphicsPixmapItem>
#include <memory>

namespace UnTech {
namespace GuiQt {

namespace Entity::EntityRomEntries {
class ResourceItem;
}

namespace Rooms {

namespace RM = UnTech::Rooms;

class EntityGroupGraphicsItem;

class EntityEntryGraphicsItem final : public QGraphicsItem {
    using EntityPixmap = UnTech::GuiQt::Entity::EntityRomEntries::EntityPixmap;

public:
    explicit EntityEntryGraphicsItem(unsigned groupIndex, unsigned entityIndex, EntityGroupGraphicsItem* parent);
    ~EntityEntryGraphicsItem() = default;

    void clearEntityPixmap();

    void updateEntity(const RM::EntityEntry& ee, const Entity::EntityRomEntries::ResourceItem& entityResourceItem);

    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    void updateBoundingRect();
    void updateSelectionOutline();

private:
    std::unique_ptr<AabbSelectionOutlineItem> _selectionOutline;

    const std::pair<size_t, size_t> _index;

    EntityPixmap _entityPixmap;
};

class EntityGroupGraphicsItem final : public QGraphicsItem {
    using EntityPixmap = UnTech::GuiQt::Entity::EntityRomEntries::EntityPixmap;

    constexpr static int Z_VALUE = 10;

public:
    EntityGroupGraphicsItem(const size_t groupIndex);
    ~EntityGroupGraphicsItem() = default;

    void updateAllEntities(const std::vector<RM::EntityEntry>& entities, const Entity::EntityRomEntries::ResourceItem& entityResourceItem);

    int nEntities() const { return _children.size(); }
    EntityEntryGraphicsItem* at(int i) { return _children.at(i); }
    const EntityEntryGraphicsItem* at(int i) const { return _children.at(i); }

    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;

private:
    const size_t _groupIndex;
    QList<EntityEntryGraphicsItem*> _children;
};
}
}
}
