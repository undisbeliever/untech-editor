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

    // Handles the user pressing escape in the MtEditableGraphicsScene.
    // Return true if the cursor is to be removed.
    // This function MUST NOT DELETE this class instance.
    // The default implementation always returns true, indicating the cursor is to be removed.
    virtual bool processEscape();

protected:
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    bool _enableClickDrag;
};
}
}
}
