/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QAction>
#include <QGraphicsObject>

namespace UnTech {
namespace GuiQt {
namespace MetaTiles {

class AbstractCursorGraphicsItem : public QGraphicsObject {
    Q_OBJECT

public:
    AbstractCursorGraphicsItem();
    ~AbstractCursorGraphicsItem() = default;

    // return true if the item has moved
    virtual bool processMouseScenePosition(const QPointF& scenePos) = 0;

    // Handles the user pressing escape in the MtEditableGraphicsScene.
    // Return true if the cursor is to be removed.
    // This function MUST NOT DELETE this class instance.
    // The default implementation always returns true, indicating the cursor is to be removed.
    virtual bool processEscape();
};

class AbstractCursorFactory : public QObject {
    Q_OBJECT

private:
    QAction* const _action;

public:
    AbstractCursorFactory(const QIcon& icon, const QString& text, QObject* parent);

    QAction* action() { return _action; }

    virtual AbstractCursorGraphicsItem* createCursor() = 0;
};
}
}
}
