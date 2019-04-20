/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
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
const QColor Style::INVALID_CURSOR_PEN_COLOR(255, 0, 0, 255);
const QColor Style::INVALID_CURSOR_BRUSH_COLOR(128, 0, 0, 128);

Style::Style(QWidget* parent)
    : QObject(parent)
    , _widget(parent)
    , _showGridAction(new QAction(tr("Show Grid"), this))
    , _showGrid(true)
{
    Q_ASSERT(_widget != nullptr);

    _showGridAction->setCheckable(true);
    _showGridAction->setChecked(_showGrid);
    _showGridAction->setShortcut(Qt::CTRL + Qt::Key_G);

    connect(_showGridAction, &QAction::toggled,
            this, &Style::setShowGrid);
}

void Style::setShowGrid(bool showGrid)
{
    if (_showGrid != showGrid) {
        _showGrid = showGrid;
        _showGridAction->setChecked(showGrid);

        emit showGridChanged();
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
