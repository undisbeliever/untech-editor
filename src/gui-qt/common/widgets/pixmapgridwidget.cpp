/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "pixmapgridwidget.h"

#include <QPainter>

using namespace UnTech;
using namespace UnTech::GuiQt;

PixmapGridWidget::PixmapGridWidget(QWidget* parent)
    : QWidget(parent)
    , _backgroundColor()
    , _gridColor(100, 100, 128, 128)
    , _cellSize(32, 32)
    , _pixmaps()
{
    updateWindowSize();
}

void PixmapGridWidget::setBackgroundColor(const QColor& color)
{
    if (_backgroundColor != color) {
        _backgroundColor = color;
        update();
    }
}

void PixmapGridWidget::setGridColor(const QColor& color)
{
    if (_gridColor != color) {
        _gridColor = color;
        update();
    }
}

void PixmapGridWidget::setCellSize(const QSize& cellSize)
{
    if (_cellSize != cellSize) {
        _cellSize = cellSize;
        updateWindowSize();
        update();
    }
}

void PixmapGridWidget::setPixmaps(const QVector<QPixmap>& pixmaps)
{
    int oldSize = _pixmaps.size();

    _pixmaps = pixmaps;

    if (_pixmaps.size() != oldSize) {
        updateWindowSize();
    }
    update();
}

bool PixmapGridWidget::hasHeightForWidth() const
{
    return true;
}

int PixmapGridWidget::heightForWidth(int width) const
{
    int nColumns = qMax(1, (width - 1) / _cellSize.width());
    int nRows = qMax(1, (_pixmaps.size() + nColumns - 1) / nColumns);

    return nRows * _cellSize.height() + 1;
}

void PixmapGridWidget::updateWindowSize()
{
    setMinimumSize(_cellSize + QSize(1, 1));

    // required for resize() to work when this widget is inside a Layout
    int n = qMax(1, _pixmaps.size());
    setMaximumSize(_cellSize * n + QSize(1, 1));

    resize(width(), heightForWidth(width()));
}

void PixmapGridWidget::paintEvent(QPaintEvent*)
{
    if (_pixmaps.empty()) {
        return;
    }

    QPainter painter(this);

    const int width = this->width() - 1;

    const int cellWidth = _cellSize.width();
    const int cellHeight = _cellSize.height();

    const int nColumns = qMax(1, width / cellWidth);
    const int nRows = (_pixmaps.size() + nColumns - 1) / nColumns;
    const int cellsInLastRow = _pixmaps.size() - ((nRows - 1) * nColumns);

    // Draw Background
    if (_backgroundColor.isValid()) {
        if (_pixmaps.size() > nColumns) {
            QRect r(0, 0, nColumns * cellWidth, (nRows - 1) * cellHeight);
            painter.fillRect(r, _backgroundColor);
        }

        QRect r(0, (nRows - 1) * cellWidth, cellsInLastRow * cellWidth, cellHeight);
        painter.fillRect(r, _backgroundColor);
    }

    // Draw Pixmaps
    {
        int x = 0;
        int y = 0;
        for (int i = 0; i < _pixmaps.size(); i++) {
            const QPixmap& pixmap = _pixmaps.at(i);

            painter.drawPixmap(x, y, cellWidth, cellHeight, pixmap);

            x += cellWidth;
            if (x + cellWidth > width) {
                x = 0;
                y += cellHeight;
            }
        }
    }

    // Draw Grid
    if (_gridColor.isValid()) {
        QPen pen(_gridColor, 1, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
        painter.setPen(pen);

        for (int y = 1; y <= nRows; y++) {
            int yPos = y * cellHeight;
            int xEnd = nColumns * cellWidth;

            if (y >= nRows) {
                xEnd = cellsInLastRow * cellWidth;
            }

            painter.drawLine(0, yPos, xEnd, yPos);
        }

        for (int x = 1; x <= nColumns; x++) {
            int xPos = x * cellWidth;
            int yEnd = nRows * cellHeight;

            if (x > cellsInLastRow) {
                if (nRows == 1) {
                    break;
                }
                yEnd -= cellWidth;
            }

            painter.drawLine(xPos, 0, xPos, yEnd);
        }
    }
}
