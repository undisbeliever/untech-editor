/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationpreviewitem.h"
#include "gui-qt/metasprite/layersettings.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QtMath>

using namespace UnTech;
using namespace UnTech::GuiQt::MetaSprite::Animation;

AnimationPreviewItem::AnimationPreviewItem(const MSA::Animation::map_t* map,
                                           QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , _animationId()
    , _state()
    , _frame()
{
    Q_ASSERT(map != nullptr);

    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);

    _state.setAnimationMap(map);
    sync();
}

void AnimationPreviewItem::setAnimation(const idstring& animationId)
{
    _animationId = animationId;
    resetAnimation();
}

void AnimationPreviewItem::processDisplayFrame()
{
    _state.processDisplayFrame();
    sync();
}

void AnimationPreviewItem::nextAnimationFrame()
{
    _state.nextAnimationFrame();
    sync();
}

void AnimationPreviewItem::resetAnimation()
{
    _state.setAnimation(_animationId);
    _state.setPositionInt(point(0, 0));
    _state.resetFrameCount();

    sync();
}

void AnimationPreviewItem::sync()
{
    point pos = _state.positionInt();
    point wrapped = wrapPosition(pos);

    if (pos != wrapped) {
        pos = wrapped;
        _state.setPositionInt(pos);
    }
    setPos(pos.x, pos.y);

    const NameReference& frame = _state.frame();
    if (_frame != frame) {
        _frame = frame;
        update();
    }
}

QRectF AnimationPreviewItem::boundingRect() const
{
    return QRectF(-8, -8, 16, 16);
}

void AnimationPreviewItem::paint(
    QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setPen(Qt::red);

    painter->drawLine(-7, -7, 7, 7);
    painter->drawLine(-7, 7, 7, -7);
}

QVariant AnimationPreviewItem::itemChange(GraphicsItemChange change,
                                          const QVariant& value)
{
    if (change == QGraphicsItem::ItemPositionChange) {
        QPointF posF = value.toPointF();
        point pos(qFloor(posF.x()), qFloor(posF.y()));
        pos = wrapPosition(pos);

        if (pos != _state.positionInt()) {
            _state.setPositionInt(pos);
        }
        return QPointF(pos.x, pos.y);
    }
    return QGraphicsItem::itemChange(change, value);
}

point AnimationPreviewItem::wrapPosition(const point& position) const
{
    point pos = position;

    if (scene()) {
        QRectF r = scene()->sceneRect();

        if (pos.x < r.left()) {
            pos.x = r.right() - 1;
        }
        else if (pos.x > r.right()) {
            pos.x = r.left() + 1;
        }
        if (pos.y < r.top()) {
            pos.y = r.bottom() - 1;
        }
        else if (pos.y > r.bottom()) {
            pos.y = r.top() + 1;
        }
    }

    return pos;
}
