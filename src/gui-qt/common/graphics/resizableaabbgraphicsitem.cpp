/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resizableaabbgraphicsitem.h"

#include <QCursor>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMoveEvent>
#include <QtMath>

using namespace UnTech::GuiQt;

ResizableAabbGraphicsItem::ResizableAabbGraphicsItem(QGraphicsItem* parent)
    : AabbGraphicsItem(parent)
    , _state(State::NONE)
{
    setAcceptHoverEvents(true);
}

QRectF ResizableAabbGraphicsItem::boundingRect() const
{
    QRectF r = AabbGraphicsItem::boundingRect();

    if (isSelected() && (flags() & ItemIsMovable)) {
        r.adjust(-THRESHOLD, -THRESHOLD, 1.0, 1.0);
    }
    return r;
}

void ResizableAabbGraphicsItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    const QPointF pos = event->pos();

    QList<QGraphicsItem*> selectedItems;
    if (scene()) {
        selectedItems = scene()->selectedItems();
    }

    int s = State::NONE;

    if (isSelected()
        && (flags() & ItemIsMovable)
        && selectedItems.size() == 1) {

        if (pos.y() <= THRESHOLD) {
            s |= State::TOP;
        }
        else if (pos.y() >= size().height() - THRESHOLD) {
            s |= State::BOTTOM;
        }

        if (pos.x() <= THRESHOLD) {
            s |= State::LEFT;
        }
        else if (pos.x() >= size().width() - THRESHOLD) {
            s |= State::RIGHT;
        }
    }

    _state = State(s);
    updateCursor();

    AabbGraphicsItem::hoverEnterEvent(event);
}

void ResizableAabbGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    _state = State::NONE;
    unsetCursor();

    AabbGraphicsItem::hoverMoveEvent(event);
}

void ResizableAabbGraphicsItem::updateCursor()
{
    if (!isSelected()) {
        unsetCursor();
        return;
    }

    switch (_state) {
    case State::TOP:
    case State::BOTTOM:
        setCursor(Qt::SizeVerCursor);
        break;

    case State::LEFT:
    case State::RIGHT:
        setCursor(Qt::SizeHorCursor);
        break;

    case State::TOP_LEFT:
    case State::BOTTOM_RIGHT:
        setCursor(Qt::SizeFDiagCursor);
        break;

    case State::TOP_RIGHT:
    case State::BOTTOM_LEFT:
        setCursor(Qt::SizeBDiagCursor);
        break;

    default:
        unsetCursor();
        return;
    }
}

void ResizableAabbGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (_state != NONE && event->button() | Qt::LeftButton) {
        const QPointF pos = event->pos();
        int x = qFloor(pos.x());
        int y = qFloor(pos.y());

        const QRect range = this->range();

        QRect r = rect();

        if (_state & State::TOP) {
            r.setTop(qBound(range.top(), r.top() + y, r.bottom()));
        }
        else if (_state & State::BOTTOM) {
            r.setHeight(qMax(1, y));
        }

        if (_state & State::LEFT) {
            r.setLeft(qBound(range.left(), r.left() + x, r.right()));
        }
        else if (_state & State::RIGHT) {
            r.setWidth(qMax(1, x));
        }

        setRect(r);
    }
    else {
        AabbGraphicsItem::mouseMoveEvent(event);
    }
}

QVariant ResizableAabbGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemSelectedHasChanged) {
        prepareGeometryChange();
        updateCursor();
    }
    return AabbGraphicsItem::itemChange(change, value);
}
