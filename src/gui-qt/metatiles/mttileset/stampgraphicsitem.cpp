/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "stampgraphicsitem.h"
#include "mtgraphicsscenes.h"
#include "mttilesetrenderer.h"
#include "mttilesetresourceitem.h"
#include "gui-qt/metatiles/style.h"
#include "models/metatiles/metatile-tileset.h"

#include <QGraphicsSceneMouseEvent>
#include <QtMath>

using namespace UnTech;
using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaTiles;

StampGraphicsItem::StampGraphicsItem(MtEditableGraphicsScene* scene)
    : AbstractCursorGraphicsItem()
    , _scene(scene)
    , _tilePosition()
    , _boundingRect()
    , _grid()
    , _tileGridPainter()
{
    Q_ASSERT(scene);

    setEnableClickDrag(true);

    connect(scene->renderer(), &MtTilesetRenderer::nMetaTilesChanged,
            this, &StampGraphicsItem::updateTileGridFragments);

    connect(scene->renderer(), &MtTilesetRenderer::pixmapChanged,
            this, &StampGraphicsItem::updateAll);

    connect(scene->style(), &Style::showGridChanged,
            this, &StampGraphicsItem::updateAll);
}

void StampGraphicsItem::setGrid(grid_t&& grid)
{
    if (_grid != grid) {
        const auto oldSize = _grid.size();

        _grid = grid;
        if (grid.size() != oldSize) {
            _boundingRect.setWidth(_grid.width() * 16);
            _boundingRect.setHeight(_grid.height() * 16);

            prepareGeometryChange();
        }

        updateTileGridFragments();
    }
}

QRectF StampGraphicsItem::boundingRect() const
{
    return _boundingRect;
}

void StampGraphicsItem::updateAll()
{
    update();
}

void StampGraphicsItem::updateTileGridFragments()
{
    _tileGridPainter.updateFragments(_scene->renderer(), _grid);
    updateAll();
}

bool StampGraphicsItem::processMouseScenePosition(const QPointF& scenePos)
{
    QPointF center(scenePos.x() - (_grid.width() - 1) * METATILE_SIZE / 2,
                   scenePos.y() - (_grid.height() - 1) * METATILE_SIZE / 2);

    point tilePos(qFloor(center.x() / METATILE_SIZE),
                  qFloor(center.y() / METATILE_SIZE));

    if (_tilePosition != tilePos) {
        _tilePosition = tilePos;
        setPos(_tilePosition.x * METATILE_SIZE, _tilePosition.y * METATILE_SIZE);

        return true;
    }

    return false;
}

void StampGraphicsItem::processClick()
{
    _scene->placeTiles(_grid, _tilePosition);
}

void StampGraphicsItem::paint(QPainter* painter,
                              const QStyleOptionGraphicsItem*, QWidget*)
{
    auto* renderer = _scene->renderer();
    unsigned nMetaTiles = renderer->nMetaTiles();
    QColor backgroundColor = renderer->backgroundColor();

    auto gridIt = _grid.cbegin();
    for (unsigned y = 0; y < _grid.height(); y++) {
        for (unsigned x = 0; x < _grid.width(); x++) {
            const auto& tile = *gridIt++;

            if (tile < nMetaTiles) {
                painter->fillRect(x * METATILE_SIZE, y * METATILE_SIZE,
                                  METATILE_SIZE, METATILE_SIZE,
                                  backgroundColor);
            }
        }
    }
    Q_ASSERT(gridIt == _grid.cend());

    auto* style = _scene->style();

    _tileGridPainter.paint(painter, renderer);

    if (style->showGrid()) {
        painter->save();

        painter->setPen(style->gridPen());
        painter->setBrush(QBrush());

        int width = _boundingRect.width();
        int height = _boundingRect.height();

        for (int x = 0; x <= width; x += 16) {
            painter->drawLine(x, 0, x, height);
        }
        for (int y = 0; y <= height; y += 16) {
            painter->drawLine(0, y, width, y);
        }

        painter->restore();
    }
}
