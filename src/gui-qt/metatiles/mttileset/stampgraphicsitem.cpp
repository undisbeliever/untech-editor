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

    connect(scene, &MtEditableGraphicsScene::cursorRectChanged,
            this, &StampGraphicsItem::updateAll);

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

QRect StampGraphicsItem::validCellsRect() const
{
    const QRect& cr = _scene->cursorRect();

    return QRect(cr.x() / METATILE_SIZE, cr.y() / METATILE_SIZE,
                 cr.width() / METATILE_SIZE, cr.height() / METATILE_SIZE);
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
    const int gridWidth = _grid.width();
    const int gridHeight = _grid.height();
    QRect validCells = validCellsRect();

    if (_tilePosition.x <= validCells.right()
        && _tilePosition.x + gridWidth - 1 >= validCells.left()
        && _tilePosition.y <= validCells.bottom()
        && _tilePosition.y + gridHeight - 1 >= validCells.top()) {

        _scene->placeTiles(_grid, _tilePosition);
    }
}

void StampGraphicsItem::paint(QPainter* painter,
                              const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->save();

    auto* renderer = _scene->renderer();
    const unsigned nMetaTiles = renderer->nMetaTiles();
    const QColor backgroundColor = renderer->backgroundColor();
    const int gridWidth = _grid.width();
    const int gridHeight = _grid.height();

    auto gridIt = _grid.cbegin();
    for (int y = 0; y < gridHeight; y++) {
        for (int x = 0; x < gridWidth; x++) {
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

    QRect validCells = validCellsRect();
    if (_tilePosition.x < validCells.left()
        || _tilePosition.x + gridWidth - 1 > validCells.right()
        || _tilePosition.y < validCells.top()
        || _tilePosition.y + gridHeight - 1 > validCells.bottom()) {

        painter->setPen(style->invalidCursorPen());
        painter->setBrush(style->invalidCursorBrush());

        for (int y = 0; y < gridHeight; y++) {
            for (int x = 0; x < gridWidth; x++) {
                if (validCells.contains(x + _tilePosition.x, y + _tilePosition.y) == false
                    && _grid.at(x, y) < nMetaTiles) {

                    painter->drawRect(x * METATILE_SIZE, y * METATILE_SIZE,
                                      METATILE_SIZE, METATILE_SIZE);
                }
            }
        }
    }

    if (_tilePosition.x <= validCells.right()
        && _tilePosition.x + gridWidth - 1 >= validCells.left()
        && _tilePosition.y <= validCells.bottom()
        && _tilePosition.y + gridHeight - 1 >= validCells.top()) {

        painter->setPen(style->validCursorPen());
        painter->setBrush(style->validCursorBrush());

        for (int y = 0; y < gridHeight; y++) {
            for (int x = 0; x < gridWidth; x++) {
                if (validCells.contains(x + _tilePosition.x, y + _tilePosition.y)
                    && _grid.at(x, y) < nMetaTiles) {

                    painter->drawRect(x * METATILE_SIZE, y * METATILE_SIZE,
                                      METATILE_SIZE, METATILE_SIZE);
                }
            }
        }
    }

    painter->restore();
}
