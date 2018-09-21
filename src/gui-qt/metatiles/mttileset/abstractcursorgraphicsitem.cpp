/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mtgraphicsscenes.h"
#include "mttilesetrenderer.h"
#include "mttilesetresourceitem.h"
#include "stampgraphicsitem.h"
#include "gui-qt/metatiles/style.h"
#include "models/metatiles/metatile-tileset.h"

#include <QGraphicsSceneMouseEvent>

using namespace UnTech;
using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaTiles;

AbstractCursorGraphicsItem::AbstractCursorGraphicsItem()
    : QGraphicsObject()
    , _enableClickDrag(false)
{
    setFlag(ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
}

void AbstractCursorGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    bool moved = processMouseScenePosition(event->scenePos());

    if (moved && _enableClickDrag && event->buttons() == Qt::LeftButton) {
        processClick();
    }
}

void AbstractCursorGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    processMouseScenePosition(event->scenePos());

    if (event->button() == Qt::LeftButton) {
        processClick();
    }
}
