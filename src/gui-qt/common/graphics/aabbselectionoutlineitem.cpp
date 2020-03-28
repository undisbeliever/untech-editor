/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "aabbselectionoutlineitem.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QTimerEvent>

using namespace UnTech::GuiQt;

AabbSelectionOutlineItem::AabbSelectionOutlineItem(QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , _rect()
    , _dashOffset(0)
{
    setZValue(SELECTION_OUTLINE_ZVALUE);

    setAcceptedMouseButtons(Qt::NoButton);

    _timer.start(100, this);
}

void AabbSelectionOutlineItem::setRect(const QRectF& rect)
{
    prepareGeometryChange();
    _rect = rect;

    update();
}

QRectF AabbSelectionOutlineItem::boundingRect() const
{
    return _rect.adjusted(-1, -1, 1, 1);
}

void AabbSelectionOutlineItem::paint(QPainter* painter,
                                     const QStyleOptionGraphicsItem*, QWidget*)
{
#if QT_VERSION >= 0x050600
    qreal pr = painter->device()->devicePixelRatioF();
#else
    qreal pr = painter->device()->devicePixelRatio();
#endif

    QPen white(Qt::white, pr, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
    white.setCosmetic(true);

    QPen black(Qt::black, pr, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
    black.setDashPattern({ 3.0 * pr, 3.0 * pr });
    black.setDashOffset(_dashOffset);
    black.setCosmetic(true);

    painter->setBrush(Qt::NoBrush);

    painter->setPen(white);
    painter->drawRect(_rect);

    painter->setPen(black);
    painter->drawRect(_rect);
}

void AabbSelectionOutlineItem::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == _timer.timerId()) {
        _dashOffset += 0.5;
        update();
    }
    else {
        QGraphicsObject::timerEvent(event);
    }
}
