/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "aabbgraphicsitem.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QtMath>

using namespace UnTech::GuiQt;

AabbGraphicsItem::AabbGraphicsItem(QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , _size(1, 1)
    , _range(0, 0, INT_MAX, INT_MAX)
    , _pen()
    , _brush(Qt::NoBrush)
    , _selectionOutline(nullptr)
{
    setFlag(ItemSendsGeometryChanges);
}

void AabbGraphicsItem::setRange(const QRect& range)
{
    if (_range != range) {
        _range = range;
        if (!_range.isValid()) {
            _range = QRect(0, 0, INT_MAX, INT_MAX);
        }

        QPointF pos = this->pos();
        int x = qFloor(pos.x());
        int y = qFloor(pos.y());

        setSize(_size);
        setPos(x, y);
    }
}

void AabbGraphicsItem::setSize(int width, int height)
{
    QPointF pos = this->pos();
    int x = qFloor(pos.x());
    int y = qFloor(pos.y());

    QSize s(qBound(1, width, _range.right() - x + 1),
            qBound(1, height, _range.bottom() - y + 1));

    if (_size != s) {
        prepareGeometryChange();
        _size = s;
        update();
        updateSelectionOutline();
    }
}

void AabbGraphicsItem::setRect(const QRect& rect)
{
    _size = QSize(0, 0);
    QGraphicsItem::setPos(rect.x(), rect.y());
    setSize(rect.width(), rect.height());
}

void AabbGraphicsItem::setRect(const urect& rect)
{
    _size = QSize(0, 0);
    QGraphicsItem::setPos(rect.x, rect.y);
    setSize(rect.width, rect.height);
}

void AabbGraphicsItem::setRect(const ms8rect& rect)
{
    _size = QSize(0, 0);
    QGraphicsItem::setPos(rect.x, rect.y);
    setSize(rect.width, rect.height);
}

void AabbGraphicsItem::setRect(const upoint& point, unsigned squareSize)
{
    _size = QSize(0, 0);
    QGraphicsItem::setPos(point.x, point.y);
    setSize(squareSize);
}

void AabbGraphicsItem::setRect(const ms8point& point, unsigned squareSize)
{
    _size = QSize(0, 0);
    QGraphicsItem::setPos(point.x, point.y);
    setSize(squareSize);
}

void AabbGraphicsItem::setPen(const QPen& pen)
{
    if (_pen != pen) {
        prepareGeometryChange();
        _pen = pen;
        update();
    }
}

void AabbGraphicsItem::setBrush(const QBrush& brush)
{
    if (_brush != brush) {
        _brush = brush;
        update();
    }
}

QRectF AabbGraphicsItem::boundingRect() const
{
    qreal m = _pen.width() / 2;
    return QRect(-m, -m, _size.width(), _size.height());
}

void AabbGraphicsItem::paint(QPainter* painter,
                             const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setPen(_pen);
    painter->setBrush(_brush);

    painter->drawRect(0, 0, _size.width(), _size.height());
}

QVariant AabbGraphicsItem::itemChange(GraphicsItemChange change,
                                      const QVariant& value)
{
    if (change == QGraphicsItem::ItemPositionChange) {
        QPointF pos = value.toPointF();
        int x = qFloor(pos.x());
        int y = qFloor(pos.y());

        return QPointF(
            qBound(_range.x(), x, _range.right() - _size.width() + 1),
            qBound(_range.y(), y, _range.bottom() - _size.height() + 1));
    }
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

void AabbGraphicsItem::updateSelectionOutline()
{
    if (_selectionOutline) {
        QRect r(0, 0, _size.width(), _size.height());

        _selectionOutline->setTransform(sceneTransform());
        _selectionOutline->setRect(r);
    }
}
