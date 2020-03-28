/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "style.h"
#include <QAction>
#include <QBrush>
#include <QPen>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaTiles;

const QColor Style::GRID_PEN_COLOR(128, 128, 128, 128);

const QColor Style::GRID_SELECTION_PEN_COLOR(0, 0, 255, 255);
const QColor Style::GRID_SELECTION_BRUSH_COLOR(0, 0, 128, 128);

const QColor Style::VALID_CURSOR_PEN_COLOR(0, 255, 0, 255);
const QColor Style::VALID_CURSOR_BRUSH_COLOR(0, 128, 0, 128);
const QColor Style::ERASER_CURSOR_PEN_COLOR(255, 128, 0, 255);
const QColor Style::ERASER_CURSOR_BRUSH_COLOR(128, 64, 0, 128);
const QColor Style::INVALID_CURSOR_PEN_COLOR(255, 0, 0, 255);
const QColor Style::INVALID_CURSOR_BRUSH_COLOR(128, 0, 0, 128);

const QColor Style::TILE_COLLISIONS_COLOR(192, 0, 192, 128);

Style::Style(QWidget* parent)
    : QObject(parent)
    , _widget(parent)
    , _showGridAction(new QAction(tr("Show Grid"), this))
    , _showTilesAction(new QAction(tr("Show Tiles"), this))
    , _showTileCollisionsAction(new QAction(tr("Show Tile Collisions"), this))
    , _showGrid(true)
    , _showTiles(true)
    , _showTileCollisions(true)
{
    Q_ASSERT(_widget != nullptr);

    _showGridAction->setCheckable(true);
    _showGridAction->setChecked(_showGrid);
    _showGridAction->setShortcut(Qt::CTRL + Qt::Key_G);

    _showTilesAction->setCheckable(true);
    _showTilesAction->setChecked(_showTiles);
    _showTilesAction->setShortcut(Qt::CTRL + Qt::Key_H);

    _showTileCollisionsAction->setCheckable(true);
    _showTileCollisionsAction->setChecked(_showTileCollisions);
    _showTileCollisionsAction->setShortcut(Qt::CTRL + Qt::Key_J);

    connect(_showGridAction, &QAction::toggled,
            this, &Style::setShowGrid);
    connect(_showTilesAction, &QAction::toggled,
            this, &Style::setShowTiles);
    connect(_showTileCollisionsAction, &QAction::toggled,
            this, &Style::setShowTileCollisions);
}

void Style::populateActions(QWidget* widget)
{
    widget->addAction(_showGridAction);
    widget->addAction(_showTilesAction);
    widget->addAction(_showTileCollisionsAction);
}

void Style::setShowGrid(bool s)
{
    if (_showGrid != s) {
        _showGrid = s;
        _showGridAction->setChecked(s);

        emit showLayersChanged();
    }
}

void Style::setShowTiles(bool s)
{
    if (_showTiles != s) {
        if (s == false) {
            // Prevent the system from drawing a blank tileset.
            setShowTileCollisions(true);
        }

        _showTiles = s;
        _showTilesAction->setChecked(s);

        emit showLayersChanged();
    }
}

void Style::setShowTileCollisions(bool s)
{
    if (_showTileCollisions != s) {
        if (s == false) {
            // Prevent the system from drawing a blank tileset.
            setShowTiles(true);
        }

        _showTileCollisions = s;
        _showTileCollisionsAction->setChecked(s);

        emit showLayersChanged();
    }
}

QPen Style::gridPen() const
{
    return createCosmeticPen(GRID_PEN_COLOR);
}

QPen Style::gridSelectionPen() const
{
    return createCosmeticPen(GRID_SELECTION_PEN_COLOR);
}

QBrush Style::gridSelectionBrush() const
{
    return QBrush(GRID_SELECTION_BRUSH_COLOR);
}

QPen Style::validCursorPen() const
{
    return createCosmeticPen(VALID_CURSOR_PEN_COLOR);
}

QBrush Style::validCursorBrush() const
{
    return QBrush(VALID_CURSOR_BRUSH_COLOR);
}

QPen Style::eraserCursorPen() const
{
    return createCosmeticPen(ERASER_CURSOR_PEN_COLOR);
}

QBrush Style::eraserCursorBrush() const
{
    return QBrush(ERASER_CURSOR_BRUSH_COLOR);
}

QPen Style::invalidCursorPen() const
{
    return createCosmeticPen(INVALID_CURSOR_PEN_COLOR);
}

QBrush Style::invalidCursorBrush() const
{
    return QBrush(INVALID_CURSOR_BRUSH_COLOR);
}

QPen Style::createCosmeticPen(const QColor& color) const
{
#if QT_VERSION >= 0x050600
    qreal pixelRatio = _widget->devicePixelRatioF();
#else
    int pixelRatio = _widget->devicePixelRatio();
#endif

    QPen pen(color, pixelRatio);
    pen.setCosmetic(true);
    return pen;
}
