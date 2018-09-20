/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QGraphicsObject>

namespace UnTech {
namespace GuiQt {
namespace MetaTiles {

class AbstractCursorGraphicsItem : public QGraphicsObject {
    Q_OBJECT

public:
    AbstractCursorGraphicsItem();
    ~AbstractCursorGraphicsItem() = default;

    bool enableClickDrag() const { return _enableClickDrag; }
    void setEnableClickDrag(bool enable) { _enableClickDrag = enable; }

    // return true if the item has moved
    virtual bool processMouseScenePosition(const QPointF& scenePos) = 0;

    virtual void processClick() = 0;

protected:
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) final;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) final;

private:
    bool _enableClickDrag;
};
}
}
}
