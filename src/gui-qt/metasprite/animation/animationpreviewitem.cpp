/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationpreviewitem.h"
#include "gui-qt/metasprite/abstractmsdocument.h"
#include "models/common/int_ms8_t.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QtMath>

using namespace UnTech;
using namespace UnTech::GuiQt::MetaSprite;
using namespace UnTech::GuiQt::MetaSprite::Animation;

AnimationPreviewItem::AnimationPreviewItem(const AbstractMsDocument* document,
                                           QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , _document(document)
    , _animationId()
    , _state()
    , _prevFrame()
    , _framePtr(nullptr)
{
    Q_ASSERT(document != nullptr);

    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);

    _state.setAnimationMap(document->animations());

    connect(_document, &AbstractMsDocument::frameAdded,
            this, &AnimationPreviewItem::onFrameAdded);
    connect(_document, &AbstractMsDocument::frameAboutToBeRemoved,
            this, &AnimationPreviewItem::onFrameAboutToBeRemoved);
    connect(_document, &AbstractMsDocument::frameDataChanged,
            this, &AnimationPreviewItem::onFrameDataAndContentsChanged);
    connect(_document, &AbstractMsDocument::frameObjectChanged,
            this, &AnimationPreviewItem::onFrameDataAndContentsChanged);
    connect(_document, &AbstractMsDocument::actionPointChanged,
            this, &AnimationPreviewItem::onFrameDataAndContentsChanged);
    connect(_document, &AbstractMsDocument::entityHitboxChanged,
            this, &AnimationPreviewItem::onFrameDataAndContentsChanged);
    connect(_document, &AbstractMsDocument::frameObjectListChanged,
            this, &AnimationPreviewItem::onFrameDataAndContentsChanged);
    connect(_document, &AbstractMsDocument::actionPointListChanged,
            this, &AnimationPreviewItem::onFrameDataAndContentsChanged);
    connect(_document, &AbstractMsDocument::entityHitboxListChanged,
            this, &AnimationPreviewItem::onFrameDataAndContentsChanged);
}

void AnimationPreviewItem::onFrameAdded()
{
    const void* framePtr = setFrame(_state.frame().name);
    if (framePtr != _framePtr) {
        _framePtr = framePtr;
        update();
    }
}

void AnimationPreviewItem::onFrameAboutToBeRemoved(const void* frame)
{
    if (_framePtr == frame) {
        _framePtr = nullptr;
        setFrame(idstring());
        update();
    }
}

void AnimationPreviewItem::onFrameDataAndContentsChanged(const void* frame)
{
    if (_framePtr == frame) {
        setFrame(_state.frame().name);
        update();
    }
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
    const void* framePtr = setFrame(frame.name);
    if (_framePtr != framePtr || _prevFrame != frame) {
        _prevFrame = frame;
        _framePtr = framePtr;
        update();
    }
}

QRectF AnimationPreviewItem::boundingRect() const
{
    return QRectF(int_ms8_t::MIN, int_ms8_t::MIN, UINT8_MAX, UINT8_MAX);
}

void AnimationPreviewItem::paint(
    QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if (_framePtr != nullptr) {
        painter->scale(_state.frame().hFlip ? -1 : 1,
                       _state.frame().vFlip ? -1 : 1);

        drawFrame(painter);
    }
    else {
        painter->setPen(Qt::red);

        painter->drawLine(-4, -4, 4, 4);
        painter->drawLine(-4, 4, 4, -4);
    }
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