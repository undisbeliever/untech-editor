/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "aabbgraphicsitem.h"

#include <QCursor>
#include <QVector>

namespace UnTech {
namespace GuiQt {

/*
 * The ResizableAabbGraphicsItem class can be resized by the mouse if the
 * ItemIsMovable flag is set and this item is the only selected item.
 */
class ResizableAabbGraphicsItem : public AabbGraphicsItem {
public:
    constexpr static qreal THRESHOLD = 1.0 / 3;

    enum State {
        NONE = 0,
        TOP = 1,
        BOTTOM = 2,
        LEFT = 4,
        RIGHT = 8,
        TOP_LEFT = TOP | LEFT,
        TOP_RIGHT = TOP | RIGHT,
        BOTTOM_LEFT = BOTTOM | LEFT,
        BOTTOM_RIGHT = BOTTOM | RIGHT
    };

public:
    explicit ResizableAabbGraphicsItem(QGraphicsItem* parent = nullptr);
    ~ResizableAabbGraphicsItem() = default;

    virtual QRectF boundingRect() const override;

protected:
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    void updateCursor();

private:
    State _state;
};
}
}
