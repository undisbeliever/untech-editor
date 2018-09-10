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
    , _tileGridPainter()
{
    Q_ASSERT(scene);

    connect(scene, &MtGraphicsScene::gridResized,
            this, &MtGridGraphicsItem::onGridResized);

    connect(scene, &MtGraphicsScene::gridChanged,
            this, &MtGridGraphicsItem::updateTileGridFragments);
    connect(scene->renderer(), &MtTilesetRenderer::nMetaTilesChanged,
            this, &MtGridGraphicsItem::updateTileGridFragments);

    connect(scene->renderer(), &MtTilesetRenderer::pixmapChanged,
            this, &MtGridGraphicsItem::updateAll);
}

QRectF MtGridGraphicsItem::boundingRect() const
{
    return _boundingRect;
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
    updateTileGridFragments();
}

void MtGridGraphicsItem::updateTileGridFragments()
{
    _tileGridPainter.updateFragments(_scene->renderer(), _scene->grid());
    updateAll();
}

void MtGridGraphicsItem::paint(QPainter* painter,
                               const QStyleOptionGraphicsItem*, QWidget*)
{
    _tileGridPainter.paint(painter, _scene->renderer());
}
