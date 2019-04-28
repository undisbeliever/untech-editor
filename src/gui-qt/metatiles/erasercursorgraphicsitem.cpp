/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "erasercursorgraphicsitem.h"
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

EraserCursorGraphicsItem::EraserCursorGraphicsItem(MtEditableGraphicsScene* scene)
    : AbstractCursorGraphicsItem()
    , _scene(scene)
    , _tileGridPainter()
    , _drawState(DrawState::STAMP)
    , _cursorPosition()
    , _cursorSize(1, 1)
    , _boundingRect()
    , _mouseScenePosition()
    , _startBoxPosition()
{
    Q_ASSERT(scene);

    setFlag(QGraphicsItem::ItemIsFocusable, true);

    updateBoundingBox();
    updateTileGridFragments();

    connect(scene, &MtEditableGraphicsScene::cursorRectChanged,
            this, &EraserCursorGraphicsItem::updateAll);

    connect(scene->renderer(), &MtTileset::MtTilesetRenderer::pixmapChanged,
            this, &EraserCursorGraphicsItem::updateAll);

    connect(scene->style(), &Style::showGridChanged,
            this, &EraserCursorGraphicsItem::updateAll);
}

bool EraserCursorGraphicsItem::setCursorPosition(const point& tilePosition)
{
    if (_cursorPosition != tilePosition) {
        _cursorPosition = tilePosition;
        setPos(_cursorPosition.x * METATILE_SIZE, _cursorPosition.y * METATILE_SIZE);
        return true;
    }

    return false;
}

void EraserCursorGraphicsItem::setCursorSize(unsigned width, unsigned height)
{
    if (_cursorSize.width != width || _cursorSize.height != height) {
        _cursorSize.width = width;
        _cursorSize.height = height;

        updateBoundingBox();
        updateTileGridFragments();
    }
}

QRect EraserCursorGraphicsItem::validCellsRect() const
{
    const QRect& cr = _scene->cursorRect();

    return QRect(cr.x() / METATILE_SIZE, cr.y() / METATILE_SIZE,
                 cr.width() / METATILE_SIZE, cr.height() / METATILE_SIZE);
}

void EraserCursorGraphicsItem::updateBoundingBox()
{
    int w = _cursorSize.width * METATILE_SIZE;
    int h = _cursorSize.height * METATILE_SIZE;

    if (int(_boundingRect.width()) != w || int(_boundingRect.height()) != h) {
        _boundingRect.setWidth(w);
        _boundingRect.setHeight(h);

        prepareGeometryChange();
    }
}

QRectF EraserCursorGraphicsItem::boundingRect() const
{
    return _boundingRect;
}

void EraserCursorGraphicsItem::updateAll()
{
    update();
}

void EraserCursorGraphicsItem::updateTileGridFragments()
{
    _tileGridPainter.generateEraseFragments(_cursorSize.width, _cursorSize.height);
    updateAll();
}

void EraserCursorGraphicsItem::paint(QPainter* painter,
                                     const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->save();

    auto* renderer = _scene->renderer();
    const QColor backgroundColor = renderer->backgroundColor();

    const int gridWidth = _cursorSize.width;
    const int gridHeight = _cursorSize.height;

    painter->fillRect(0, 0,
                      gridWidth * METATILE_SIZE, gridHeight * METATILE_SIZE,
                      backgroundColor);

    auto* style = _scene->style();

    _tileGridPainter.paint(painter, renderer);

    QRect validCells = validCellsRect();
    if (_cursorPosition.x < validCells.left()
        || _cursorPosition.x + gridWidth - 1 > validCells.right()
        || _cursorPosition.y < validCells.top()
        || _cursorPosition.y + gridHeight - 1 > validCells.bottom()) {

        painter->setPen(style->invalidCursorPen());
        painter->setBrush(style->invalidCursorBrush());

        for (int y = 0; y < gridHeight; y++) {
            for (int x = 0; x < gridWidth; x++) {
                if (validCells.contains(x + _cursorPosition.x, y + _cursorPosition.y) == false) {
                    painter->drawRect(x * METATILE_SIZE, y * METATILE_SIZE,
                                      METATILE_SIZE, METATILE_SIZE);
                }
            }
        }
    }

    if (_cursorPosition.x <= validCells.right()
        && _cursorPosition.x + gridWidth - 1 >= validCells.left()
        && _cursorPosition.y <= validCells.bottom()
        && _cursorPosition.y + gridHeight - 1 >= validCells.top()) {

        painter->setPen(style->eraserCursorPen());
        painter->setBrush(style->eraserCursorBrush());

        for (int y = 0; y < gridHeight; y++) {
            for (int x = 0; x < gridWidth; x++) {
                if (validCells.contains(x + _cursorPosition.x, y + _cursorPosition.y)) {

                    painter->drawRect(x * METATILE_SIZE, y * METATILE_SIZE,
                                      METATILE_SIZE, METATILE_SIZE);
                }
            }
        }
    }

    painter->restore();
}

bool EraserCursorGraphicsItem::processMouseScenePosition(const QPointF& scenePos)
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

bool EraserCursorGraphicsItem::processNormalMousePosition(const QPointF& scenePos)
{
    QPointF center(scenePos.x() - (_cursorSize.width - 1) * METATILE_SIZE / 2,
                   scenePos.y() - (_cursorSize.height - 1) * METATILE_SIZE / 2);

    point tilePos(qFloor(center.x() / METATILE_SIZE),
                  qFloor(center.y() / METATILE_SIZE));

    return setCursorPosition(tilePos);
}

bool EraserCursorGraphicsItem::processDrawBoxMousePosition(const QPointF& scenePos)
{
    point mousePosition(qFloor(scenePos.x()) / METATILE_SIZE,
                        qFloor(scenePos.y()) / METATILE_SIZE);

    switch (_drawState) {
    case DrawState::STAMP:
    case DrawState::DRAW_BOX:
        return setCursorPosition(mousePosition);

    case DrawState::DRAW_BOX_FIRST_CLICK:
    case DrawState::DRAW_BOX_SECOND_CLICK:
    case DrawState::DRAW_BOX_DRAGING: {
        // In the middle of drawing the box
        point topLeft(qMin(mousePosition.x, _startBoxPosition.x),
                      qMin(mousePosition.y, _startBoxPosition.y));

        unsigned width = qMax(mousePosition.x, _startBoxPosition.x) - topLeft.x + 1;
        unsigned height = qMax(mousePosition.y, _startBoxPosition.y) - topLeft.y + 1;

        bool changed = setCursorPosition(topLeft);

        if (_cursorSize.width != width || _cursorSize.height != height) {
            setCursorSize(width, height);
            setPos(_cursorPosition.x * METATILE_SIZE, _cursorPosition.y * METATILE_SIZE);

            changed = true;
        }
        return changed;
    }
    }

    return false;
}

void EraserCursorGraphicsItem::resetDrawState()
{
    if (_drawState != DrawState::STAMP) {
        _drawState = DrawState::STAMP;

        processNormalMousePosition(_mouseScenePosition);

        setCursorSize(1, 1);
        updateBoundingBox();
        updateTileGridFragments();
    }
}

void EraserCursorGraphicsItem::updateDrawState(Qt::KeyboardModifiers modifiers)
{
    if (modifiers & Qt::ShiftModifier) {
        // shift pressed
        if (_drawState == DrawState::STAMP) {
            _drawState = DrawState::DRAW_BOX;
            setCursorSize(1, 1);

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

bool EraserCursorGraphicsItem::processEscape()
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

void EraserCursorGraphicsItem::keyPressEvent(QKeyEvent* event)
{
    updateDrawState(event->modifiers());
}

void EraserCursorGraphicsItem::keyReleaseEvent(QKeyEvent* event)
{
    updateDrawState(event->modifiers());
}

void EraserCursorGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    updateDrawState(event->modifiers());

    bool moved = processMouseScenePosition(event->scenePos());

    switch (_drawState) {
    case DrawState::STAMP:
        if (moved && event->buttons() == Qt::LeftButton) {
            placeTiles(false);
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

void EraserCursorGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    updateDrawState(event->modifiers());

    processMouseScenePosition(event->scenePos());

    if (event->button() & Qt::LeftButton) {
        switch (_drawState) {
        case DrawState::STAMP:
            placeTiles(true);
            break;

        case DrawState::DRAW_BOX:
            _startBoxPosition = _cursorPosition;
            _drawState = DrawState::DRAW_BOX_FIRST_CLICK;
            break;

        case DrawState::DRAW_BOX_FIRST_CLICK:
            _drawState = DrawState::DRAW_BOX_SECOND_CLICK;
            break;

        case DrawState::DRAW_BOX_SECOND_CLICK:
            placeTiles(true);
            _drawState = DrawState::DRAW_BOX;
            setCursorSize(1, 1);
            processMouseScenePosition(_mouseScenePosition);
            break;

        case DrawState::DRAW_BOX_DRAGING:
            break;
        }
    }
}

void EraserCursorGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
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
            placeTiles(true);
            _drawState = DrawState::DRAW_BOX;
            setCursorSize(1, 1);
            processMouseScenePosition(_mouseScenePosition);
            break;
        }
    }
}

void EraserCursorGraphicsItem::placeTiles(bool firstClick)
{
    const QRect validCells = validCellsRect();

    if (_cursorPosition.x <= validCells.right()
        && int(_cursorPosition.x + _cursorSize.width - 1) >= validCells.left()
        && _cursorPosition.y <= validCells.bottom()
        && int(_cursorPosition.y + _cursorSize.height - 1) >= validCells.top()) {

        _scene->placeTiles(UnTech::grid<uint16_t>(_cursorSize, 0), _cursorPosition, tr("Erase"), firstClick);
    }
}

EraserCursorFactory::EraserCursorFactory(MtEditableGraphicsScene* scene)
    : AbstractCursorFactory(QIcon(":/icons/mt-eraser-cursor.svg"), tr("Eraser"), scene)
    , _scene(scene)
{
}

EraserCursorGraphicsItem* EraserCursorFactory::createCursor()
{
    return new EraserCursorGraphicsItem(_scene);
}
