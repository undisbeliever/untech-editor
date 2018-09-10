/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mtgridgraphicsitem.h"
#include "mtgraphicsscenes.h"
#include "mttilesetrenderer.h"
#include "mttilesetresourceitem.h"
#include "models/metatiles/metatile-tileset.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaTiles;

MtGridGraphicsItem::MtGridGraphicsItem(MtGraphicsScene* scene)
    : QGraphicsObject()
    , _scene(scene)
    , _boundingRect()
{
    Q_ASSERT(scene);

    connect(scene, &MtGraphicsScene::gridChanged,
            this, &MtGridGraphicsItem::updateAll);
    connect(scene, &MtGraphicsScene::gridResized,
            this, &MtGridGraphicsItem::onGridResized);

    connect(scene->renderer(), &MtTilesetRenderer::pixmapChanged,
            this, &MtGridGraphicsItem::updateAll);
}

QRectF MtGridGraphicsItem::boundingRect() const
{
    return _boundingRect;
}

void MtGridGraphicsItem::paint(QPainter* painter,
                               const QStyleOptionGraphicsItem*, QWidget*)
{
    _scene->renderer()->drawGridTiles(painter, _scene->grid());
}

void MtGridGraphicsItem::updateAll()
{
    update();
}

void MtGridGraphicsItem::onGridResized()
{
    const auto& grid = _scene->grid();

    _boundingRect.setWidth(grid.width() * 16);
    _boundingRect.setHeight(grid.height() * 16);

    prepareGeometryChange();
    updateAll();
}
