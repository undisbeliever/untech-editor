/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "entitygraphicsitem.h"
#include "gui-qt/entity/entity-rom-entries/resourceitem.h"
#include <QGraphicsScene>
#include <QPainter>

using namespace UnTech;
using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Rooms;

EntityEntryGraphicsItem::EntityEntryGraphicsItem(unsigned groupIndex, unsigned entityIndex, EntityGroupGraphicsItem* parent)
    : QGraphicsItem(parent)
    , _index(groupIndex, entityIndex)
    , _entityPixmap()
{
    Q_ASSERT(parent);

    setFlag(ItemIsSelectable);
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
}

QRectF EntityEntryGraphicsItem::boundingRect() const
{
    return _entityPixmap.boundingBox;
}

void EntityEntryGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->drawPixmap(-_entityPixmap.origin, _entityPixmap.pixmap);
}

void EntityEntryGraphicsItem::updateEntity(const RM::EntityEntry& ee,
                                           const Entity::EntityRomEntries::ResourceItem& entityResourceItem)
{
    static const EntityPixmap INVALID_ENTITY_PIXMAP = []() {
        constexpr int SIZE = 24;

        QPixmap p(SIZE, SIZE);
        p.fill(QColor(255, 0, 0, 128));

        return EntityPixmap{
            .pixmap = std::move(p),
            .name = QString(),
            .origin = QPoint(SIZE / 2, SIZE / 2),
            .boundingBox = QRectF(-SIZE / 2, -SIZE / 2, SIZE, SIZE)
        };
    }();

    const auto& ep = entityResourceItem.findEntityPixmap(ee.entityId).value_or(INVALID_ENTITY_PIXMAP);

    if (_entityPixmap.boundingBox != ep.boundingBox) {
        prepareGeometryChange();
    }

    _entityPixmap = ep;

    update();
    updateSelectionOutline();

    setPos(ee.position.x, ee.position.y);

    if (ee.name.isValid()) {
        setToolTip(QStringLiteral("%1 (%2)").arg(ep.name).arg(QString::fromStdString(ee.name)));
    }
    else if (ep.name.isEmpty() == false) {
        setToolTip(ep.name);
    }
    else {
        setToolTip(QString::fromStdString(ee.entityId));
    }
}

QVariant EntityEntryGraphicsItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionHasChanged) {
        updateSelectionOutline();
    }
    if (change == ItemSelectedHasChanged) {
        if (value.toBool()) {
            if (_selectionOutline == nullptr && scene()) {
                _selectionOutline = std::make_unique<AabbSelectionOutlineItem>();
                scene()->addItem(_selectionOutline.get());
                updateSelectionOutline();
            }
        }
        else {
            _selectionOutline = nullptr;
        }
    }
    return QGraphicsItem::itemChange(change, value);
}

void EntityEntryGraphicsItem::updateSelectionOutline()
{
    if (_selectionOutline) {
        _selectionOutline->setTransform(sceneTransform());
        _selectionOutline->setRect(_entityPixmap.boundingBox);
    }
}

EntityGroupGraphicsItem::EntityGroupGraphicsItem(const size_t groupIndex)
    : QGraphicsItem()
    , _groupIndex(groupIndex)
    , _children()
{
    setFlag(ItemHasNoContents);

    setZValue(Z_VALUE);
}

void EntityGroupGraphicsItem::updateAllEntities(const std::vector<RM::EntityEntry>& entities,
                                                const Entity::EntityRomEntries::ResourceItem& entityResourceItem)
{
    while (unsigned(_children.size()) > entities.size()) {
        delete _children.takeLast();
    }
    for (int i = 0; i < _children.size(); i++) {
        _children.at(i)->updateEntity(entities.at(i), entityResourceItem);
    }
    for (size_t i = _children.size(); i < entities.size(); i++) {
        auto* item = new EntityEntryGraphicsItem(_groupIndex, i, this);

        item->updateEntity(entities.at(i), entityResourceItem);

        _children.append(item);
    }
    Q_ASSERT(size_t(_children.size()) == entities.size());
}

QRectF EntityGroupGraphicsItem::boundingRect() const
{
    return QRectF{};
}

void EntityGroupGraphicsItem::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*)
{
}
