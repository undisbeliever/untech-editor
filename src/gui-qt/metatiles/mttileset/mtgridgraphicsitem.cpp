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

#include <QGraphicsSceneMouseEvent>

using namespace UnTech;
using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaTiles;

MtGridGraphicsItem::MtGridGraphicsItem(MtGraphicsScene* scene)
    : QGraphicsObject()
    , _scene(scene)
    , _boundingRect()
    , _tileGridPainter()
    , _enableMouseSelection(true)
{
    Q_ASSERT(scene);

    connect(scene, &MtGraphicsScene::gridResized,
            this, &MtGridGraphicsItem::onGridResized);

    connect(scene, &MtGraphicsScene::gridChanged,
            this, &MtGridGraphicsItem::updateTileGridFragments);
    connect(scene->renderer(), &MtTilesetRenderer::nMetaTilesChanged,
            this, &MtGridGraphicsItem::updateTileGridFragments);

    connect(scene, &MtGraphicsScene::gridSelectionChanged,
            this, &MtGridGraphicsItem::updateAll);

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

    const auto& sel = _scene->gridSelection();
    if (!sel.empty()) {
        painter->save();

        // ::TODO add style class::
        QPen pen(QColor(0, 0, 255, 255), 1);
        pen.setCosmetic(true);
        painter->setPen(pen);

        painter->setBrush(QColor(0, 0, 128, 128));

        for (const upoint& p : sel) {
            painter->drawRect(p.x * 16, p.y * 16, 16, 16);
        }

        painter->restore();
    }
}

upoint MtGridGraphicsItem::positionToGridCell(const QPointF& pos)
{
    return upoint(pos.x() / 16, pos.y() / 16);
}

void MtGridGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (_enableMouseSelection == false) {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        upoint cell = positionToGridCell(event->pos());

        if (event->modifiers() == Qt::ControlModifier) {
            // When control clicked, toggle cell selection

            upoint_vectorset sel = _scene->gridSelection();
            auto it = sel.find(cell);
            if (it == sel.end()) {
                sel.insert(cell);
            }
            else {
                sel.erase(it);
            }
            _scene->setGridSelection(std::move(sel));
        }
        else {
            // No Modifiers

            upoint_vectorset sel = { cell };
            _scene->setGridSelection(std::move(sel));
        }
    }
}
