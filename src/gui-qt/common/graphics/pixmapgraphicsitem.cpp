/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "pixmapgraphicsitem.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QtMath>

using namespace UnTech::GuiQt;

PixmapGraphicsItem::PixmapGraphicsItem(QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , _selectionOutline(nullptr)
    , _pixmap()
    , _range(0, 0, INT_MAX, INT_MAX)
    , _hFlip(false)
    , _vFlip(false)
{
    setFlag(ItemSendsGeometryChanges);
}

void PixmapGraphicsItem::setPixmap(const QPixmap& pixmap)
{
    prepareGeometryChange();

    _pixmap = pixmap;

    setPos(validatePosition(pos()));
    update();
    updateSelectionOutline();
}

void PixmapGraphicsItem::setRange(const QRect& range)
{
    if (_range != range) {
        _range = range;
        if (!_range.isValid()) {
            _range = QRect(0, 0, INT_MAX, INT_MAX);
        }

        setPos(validatePosition(pos()));
    }
}

void PixmapGraphicsItem::setFlip(bool hFlip, bool vFlip)
{
    if (_hFlip != hFlip || _vFlip != vFlip) {
        _hFlip = hFlip;
        _vFlip = vFlip;

        update();
    }
}

QRectF PixmapGraphicsItem::boundingRect() const
{
    return QRectF(0, 0, _pixmap.width(), _pixmap.height());
}

void PixmapGraphicsItem::paint(QPainter* painter,
                               const QStyleOptionGraphicsItem*, QWidget*)
{
    int halfWidth = _pixmap.width() / 2;
    int halfHeight = _pixmap.height() / 2;

    painter->translate(halfWidth, halfHeight);
    painter->scale(_hFlip ? -1 : 1,
                   _vFlip ? -1 : 1);

    painter->drawPixmap(-halfWidth, -halfHeight, _pixmap);
}

QVariant PixmapGraphicsItem::itemChange(GraphicsItemChange change,
                                        const QVariant& value)
{
    if (change == QGraphicsItem::ItemPositionChange) {
        return validatePosition(value.toPointF());
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

void PixmapGraphicsItem::updateSelectionOutline()
{
    if (_selectionOutline) {
        QRectF r(0, 0, _pixmap.width(), _pixmap.height());

        _selectionOutline->setTransform(sceneTransform());
        _selectionOutline->setRect(r);
    }
}

QPoint PixmapGraphicsItem::validatePosition(const QPointF& pos) const
{
    int x = qFloor(pos.x());
    int y = qFloor(pos.y());

    return QPoint(
        qBound(_range.x(), x, _range.right() - _pixmap.width() + 1),
        qBound(_range.y(), y, _range.bottom() - _pixmap.height() + 1));
}
