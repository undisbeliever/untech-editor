/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tilecursorgraphicsitem.h"
#include "mtgraphicsscenes.h"
#include "style.h"
#include "gui-qt/metatiles/mttileset/mttilesetrenderer.h"
#include "gui-qt/metatiles/mttileset/resourceitem.h"
#include "models/metatiles/metatile-tileset.h"

#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QtMath>

using namespace UnTech;
using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaTiles;

TileCursorGraphicsItem::TileCursorGraphicsItem(MtEditableGraphicsScene* scene)
    : AbstractCursorGraphicsItem()
    , _scene(scene)
    , _tileGridPainter()
    , _drawState(DrawState::STAMP)
    , _tilePosition()
    , _boundingRect()
    , _mouseScenePosition()
    , _boxGrid()
    , _startBoxPosition()
{
    Q_ASSERT(scene);

    setFlag(QGraphicsItem::ItemIsFocusable, true);

    onTileCursorGridChanged();

    connect(scene, &MtEditableGraphicsScene::cursorRectChanged,
            this, &TileCursorGraphicsItem::updateAll);

    connect(scene, &MtEditableGraphicsScene::tileCursorGridChanged,
            this, &TileCursorGraphicsItem::onTileCursorGridChanged);

    connect(scene->renderer(), &MtTileset::MtTilesetRenderer::pixmapChanged,
            this, &TileCursorGraphicsItem::updateAll);

    connect(scene->style(), &Style::showGridChanged,
            this, &TileCursorGraphicsItem::updateAll);
}

bool TileCursorGraphicsItem::setTilePosition(const point& tilePosition)
{
    if (_tilePosition != tilePosition) {
        _tilePosition = tilePosition;
        setPos(_tilePosition.x * METATILE_SIZE, _tilePosition.y * METATILE_SIZE);
        return true;
    }

    return false;
}

void TileCursorGraphicsItem::onTileCursorGridChanged()
{
    Q_ASSERT(_scene->tileCursorGrid().empty() == false);

    resetDrawState();
    updateBoundingBox();
    updateTileGridFragments();
}

const TileCursorGraphicsItem::selection_grid_t& TileCursorGraphicsItem::activeGrid() const
{
    if (_drawState == DrawState::STAMP) {
        return _scene->tileCursorGrid();
    }
    else {
        return _boxGrid;
    }
}

QRect TileCursorGraphicsItem::validCellsRect() const
{
    const QRect& cr = _scene->cursorRect();

    return QRect(cr.x() / METATILE_SIZE, cr.y() / METATILE_SIZE,
                 cr.width() / METATILE_SIZE, cr.height() / METATILE_SIZE);
}

void TileCursorGraphicsItem::updateBoundingBox()
{
    const selection_grid_t& grid = activeGrid();

    int w = grid.width() * METATILE_SIZE;
    int h = grid.height() * METATILE_SIZE;

    if (int(_boundingRect.width()) != w || int(_boundingRect.height()) != h) {
        _boundingRect.setWidth(w);
        _boundingRect.setHeight(h);

        prepareGeometryChange();
    }
}

QRectF TileCursorGraphicsItem::boundingRect() const
{
    return _boundingRect;
}

void TileCursorGraphicsItem::updateAll()
{
    update();
}

void TileCursorGraphicsItem::updateTileGridFragments()
{
    _tileGridPainter.updateFragments(activeGrid());
    updateAll();
}

void TileCursorGraphicsItem::paint(QPainter* painter,
                                   const QStyleOptionGraphicsItem*, QWidget*)
{
    constexpr unsigned N_METATILES = UnTech::MetaTiles::N_METATILES;

    painter->save();

    auto* renderer = _scene->renderer();
    const QColor backgroundColor = renderer->backgroundColor();

    const selection_grid_t& grid = activeGrid();
    const int gridWidth = grid.width();
    const int gridHeight = grid.height();

    auto gridIt = grid.cbegin();
    for (int y = 0; y < gridHeight; y++) {
        for (int x = 0; x < gridWidth; x++) {
            const auto& tile = *gridIt++;

            if (tile < N_METATILES) {
                painter->fillRect(x * METATILE_SIZE, y * METATILE_SIZE,
                                  METATILE_SIZE, METATILE_SIZE,
                                  backgroundColor);
            }
        }
    }
    Q_ASSERT(gridIt == grid.cend());

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
                    && grid.at(x, y) < N_METATILES) {

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
                    && grid.at(x, y) < N_METATILES) {

                    painter->drawRect(x * METATILE_SIZE, y * METATILE_SIZE,
                                      METATILE_SIZE, METATILE_SIZE);
                }
            }
        }
    }

    painter->restore();
}

bool TileCursorGraphicsItem::processMouseScenePosition(const QPointF& scenePos)
{
    _mouseScenePosition = scenePos;

    switch (_drawState) {
    case DrawState::STAMP:
    case DrawState::DRAW_BOX:
        return processNormalMousePosition(scenePos);

    case DrawState::DRAW_BOX_FIRST_CLICK:
    case DrawState::DRAW_BOX_SECOND_CLICK:
    case DrawState::DRAW_BOX_DRAGING:
        return processDrawBoxMousePosition(scenePos);
    }

    return false;
}

bool TileCursorGraphicsItem::processNormalMousePosition(const QPointF& scenePos)
{
    const auto& grid = activeGrid();

    QPointF center(scenePos.x() - (grid.width() - 1) * METATILE_SIZE / 2,
                   scenePos.y() - (grid.height() - 1) * METATILE_SIZE / 2);

    point tilePos(qFloor(center.x() / METATILE_SIZE),
                  qFloor(center.y() / METATILE_SIZE));

    return setTilePosition(tilePos);
}

bool TileCursorGraphicsItem::processDrawBoxMousePosition(const QPointF& scenePos)
{
    point mousePosition(qFloor(scenePos.x()) / METATILE_SIZE,
                        qFloor(scenePos.y()) / METATILE_SIZE);

    switch (_drawState) {
    case DrawState::STAMP:
    case DrawState::DRAW_BOX:
        return setTilePosition(mousePosition);

    case DrawState::DRAW_BOX_FIRST_CLICK:
    case DrawState::DRAW_BOX_SECOND_CLICK:
    case DrawState::DRAW_BOX_DRAGING: {
        // In the middle of drawing the box
        point topLeft(qMin(mousePosition.x, _startBoxPosition.x),
                      qMin(mousePosition.y, _startBoxPosition.y));

        unsigned width = qMax(mousePosition.x, _startBoxPosition.x) - topLeft.x + 1;
        unsigned height = qMax(mousePosition.y, _startBoxPosition.y) - topLeft.y + 1;

        bool changed = setTilePosition(topLeft);

        if (_boxGrid.width() != width || _boxGrid.height() != height) {
            createBoxGrid(width, height);
            setPos(_tilePosition.x * METATILE_SIZE, _tilePosition.y * METATILE_SIZE);

            changed = true;
        }
        return changed;
    }
    }

    return false;
}

void TileCursorGraphicsItem::resetDrawState()
{
    if (_drawState != DrawState::STAMP) {
        _drawState = DrawState::STAMP;

        processNormalMousePosition(_mouseScenePosition);

        updateBoundingBox();
        updateTileGridFragments();
    }
}

void TileCursorGraphicsItem::updateDrawState(Qt::KeyboardModifiers modifiers)
{
    if (modifiers & Qt::ShiftModifier) {
        // shift pressed
        if (_drawState == DrawState::STAMP) {
            _drawState = DrawState::DRAW_BOX;
            createBoxGrid(1, 1);

            processNormalMousePosition(_mouseScenePosition);
        }
    }
    else {
        // shift not pressed
        if (_drawState == DrawState::DRAW_BOX) {
            resetDrawState();

            processNormalMousePosition(_mouseScenePosition);
        }
    }
}

bool TileCursorGraphicsItem::processEscape()
{
    // If in the middle of drawing a box using two clicks, them stop drawing
    // the box, otherwise remove cursor.

    switch (_drawState) {
    case DrawState::STAMP:
    case DrawState::DRAW_BOX:
    case DrawState::DRAW_BOX_DRAGING:
        return true;

    case DrawState::DRAW_BOX_FIRST_CLICK:
    case DrawState::DRAW_BOX_SECOND_CLICK:
        resetDrawState();
        return false;
    }
    return true;
}

void TileCursorGraphicsItem::keyPressEvent(QKeyEvent* event)
{
    updateDrawState(event->modifiers());
}

void TileCursorGraphicsItem::keyReleaseEvent(QKeyEvent* event)
{
    updateDrawState(event->modifiers());
}

void TileCursorGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    updateDrawState(event->modifiers());

    bool moved = processMouseScenePosition(event->scenePos());

    switch (_drawState) {
    case DrawState::STAMP:
        if (moved && event->buttons() == Qt::LeftButton) {
            processClick();
        }
        break;

    case DrawState::DRAW_BOX_FIRST_CLICK:
        if (moved) {
            _drawState = DrawState::DRAW_BOX_DRAGING;
        }
        break;

    case DrawState::DRAW_BOX:
    case DrawState::DRAW_BOX_SECOND_CLICK:
    case DrawState::DRAW_BOX_DRAGING:
        break;
    }
}

void TileCursorGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    updateDrawState(event->modifiers());

    processMouseScenePosition(event->scenePos());

    if (event->button() & Qt::LeftButton) {
        switch (_drawState) {
        case DrawState::STAMP:
            processClick();
            break;

        case DrawState::DRAW_BOX:
            _startBoxPosition = _tilePosition;
            _drawState = DrawState::DRAW_BOX_FIRST_CLICK;
            break;

        case DrawState::DRAW_BOX_FIRST_CLICK:
            _drawState = DrawState::DRAW_BOX_SECOND_CLICK;
            break;

        case DrawState::DRAW_BOX_SECOND_CLICK:
            processClick();
            _drawState = DrawState::DRAW_BOX;
            createBoxGrid(1, 1);
            processMouseScenePosition(_mouseScenePosition);
            break;

        case DrawState::DRAW_BOX_DRAGING:
            break;
        }
    }
}

void TileCursorGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    updateDrawState(event->modifiers());

    processMouseScenePosition(event->scenePos());

    if (event->button() & Qt::LeftButton) {
        switch (_drawState) {
        case DrawState::STAMP:
        case DrawState::DRAW_BOX:
        case DrawState::DRAW_BOX_FIRST_CLICK:
        case DrawState::DRAW_BOX_SECOND_CLICK:
            break;

        case DrawState::DRAW_BOX_DRAGING:
            processClick();
            _drawState = DrawState::DRAW_BOX;
            createBoxGrid(1, 1);
            processMouseScenePosition(_mouseScenePosition);
            break;
        }
    }
}

void TileCursorGraphicsItem::processClick()
{
    const QRect validCells = validCellsRect();
    const selection_grid_t& grid = activeGrid();
    const int gridWidth = grid.width();
    const int gridHeight = grid.height();

    if (_tilePosition.x <= validCells.right()
        && _tilePosition.x + gridWidth - 1 >= validCells.left()
        && _tilePosition.y <= validCells.bottom()
        && _tilePosition.y + gridHeight - 1 >= validCells.top()) {

        _scene->placeTiles(grid, _tilePosition, tr("Place Tiles"));
    }
}

void TileCursorGraphicsItem::createBoxGrid(unsigned width, unsigned height)
{
    Q_ASSERT(_drawState != DrawState::STAMP);

    const selection_grid_t& sGrid = _scene->tileCursorGrid();

    Q_ASSERT(width > 0);
    Q_ASSERT(height > 0);
    Q_ASSERT(sGrid.empty() == false);

    const unsigned sgLastX = sGrid.width() - 1;
    const unsigned sgLastY = sGrid.height() - 1;

    if (_boxGrid.width() != width || _boxGrid.height() != height) {
        _boxGrid = selection_grid_t(width, height);
    }

    // set four corners
    _boxGrid.set(width - 1, height - 1, sGrid.at(sgLastX, sgLastY));
    _boxGrid.set(width - 1, 0, sGrid.at(sgLastX, 0));
    _boxGrid.set(0, height - 1, sGrid.at(0, sgLastY));
    _boxGrid.set(0, 0, sGrid.at(0, 0));

    unsigned sgX;
    auto resetSgX = [&]() { sgX = qMin(1U, sgLastX); };
    auto incSgX = [&]() {
        sgX++;
        if (sgX >= sgLastX) {
            sgX = qMin(1U, sgLastX);
        }
    };

    unsigned sgY;
    auto resetSgY = [&]() { sgY = qMin(1U, sgLastY); };
    auto incSgY = [&]() {
        sgY++;
        if (sgY >= sgLastY) {
            sgY = qMin(1U, sgLastY);
        }
    };

    // draw top/bottom row
    resetSgX();
    for (unsigned x = 1; x < width - 1; x++) {
        if (height > 1) {
            _boxGrid.set(x, height - 1, sGrid.at(sgX, sgLastY));
        }
        _boxGrid.set(x, 0, sGrid.at(sgX, 0));

        incSgX();
    }

    // draw left/right columns
    resetSgY();
    for (unsigned y = 1; y < height - 1; y++) {
        if (width > 1) {
            _boxGrid.set(width - 1, y, sGrid.at(sgLastX, sgY));
        }
        _boxGrid.set(0, y, sGrid.at(0, sgY));

        incSgY();
    }

    // draw middle
    if (width >= 3 && height >= 3) {
        resetSgX();
        resetSgY();
        for (unsigned y = 1; y < height - 1; y++) {
            for (unsigned x = 1; x < width - 1; x++) {
                _boxGrid.set(x, y, sGrid.at(sgX, sgY));
                incSgX();
            }
            incSgY();
        }
    }

    updateBoundingBox();
    updateTileGridFragments();
}

TileCursorFactory::TileCursorFactory(MtEditableGraphicsScene* scene)
    : AbstractCursorFactory(QIcon(":/icons/mt-tile-cursor.svg"), tr("Place Tiles"), scene)
    , _scene(scene)
{
}

TileCursorGraphicsItem* TileCursorFactory::createCursor()
{
    if (_scene->tileCursorGrid().empty()) {
        return nullptr;
    }

    return new TileCursorGraphicsItem(_scene);
}
