/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mtgraphicsscenes.h"
#include "style.h"
#include "tilecursorgraphicsitem.h"
#include "gui-qt/metatiles/mttileset/mttilesetrenderer.h"
#include "gui-qt/metatiles/mttileset/resourceitem.h"
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

bool AbstractCursorGraphicsItem::processEscape()
{
    return true;
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
