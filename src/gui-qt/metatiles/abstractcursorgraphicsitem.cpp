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
{
    setFlag(ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
}

bool AbstractCursorGraphicsItem::processEscape()
{
    return true;
}

AbstractCursorFactory::AbstractCursorFactory(const QIcon& icon, const QString& text, QObject* parent)
    : QObject(parent)
    , _action(new QAction(icon, text, this))
{
    _action->setCheckable(true);
    _action->setChecked(false);
}
