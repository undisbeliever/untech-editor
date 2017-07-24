/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "pixmapgridwidget.h"

#include <QPaintEvent>
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

void PixmapGridWidget::paintEvent(QPaintEvent* event)
{
    if (_pixmaps.empty()) {
        return;
    }

    QPainter painter(this);

    const int cellWidth = _cellSize.width();
    const int cellHeight = _cellSize.height();

    const int nColumns = qMax(1, (width() - 1) / cellWidth);
    const int nRows = (_pixmaps.size() + nColumns - 1) / nColumns;
    const int lastRow = nRows - 1;
    const int cellsInLastRow = _pixmaps.size() - (lastRow * nColumns);

    const QRect& toUpdate = event->rect();
    const int xStart = qMax(0, toUpdate.left() / cellWidth);
    const int yStart = qMax(0, toUpdate.top() / cellHeight);
    const int xEnd = qMin(nColumns - 1, (toUpdate.right() + cellWidth - 1) / cellWidth);
    const int yEnd = qMin(nRows - 1, (toUpdate.bottom() + cellHeight - 1) / cellHeight);

    // Draw Background
    if (_backgroundColor.isValid()) {
        if (_pixmaps.size() > nColumns && yStart < lastRow) {
            QRect r(0, 0, nColumns * cellWidth, lastRow * cellHeight);
            painter.fillRect(r, _backgroundColor);
        }

        if (lastRow == yEnd) {
            QRect r(0, lastRow * cellWidth, cellsInLastRow * cellWidth, cellHeight);
            painter.fillRect(r, _backgroundColor);
        }
    }

    // Draw Pixmaps
    {
        for (int y = yStart; y <= yEnd; y++) {
            int yPos = y * cellHeight;
            int xPos = xStart * cellWidth;
            int index = y * nColumns + xStart;

            for (int x = xStart; x <= xEnd; x++) {
                if (index >= _pixmaps.size()) {
                    break;
                }

                const QPixmap& pixmap = _pixmaps.at(index);
                painter.drawPixmap(xPos, yPos, cellWidth, cellHeight, pixmap);

                xPos += cellWidth;
                index++;
            }
        }
    }

    // Draw Grid
    if (_gridColor.isValid()) {
        QPen pen(_gridColor, 1, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
        painter.setPen(pen);

        for (int y = qMax(1, yStart); y <= yEnd + 1; y++) {
            int yPos = y * cellHeight;
            int x1 = xStart * cellWidth;
            int x2 = (xEnd + 1) * cellWidth;

            if (y > lastRow) {
                x2 = cellsInLastRow * cellWidth;
            }

            painter.drawLine(x1, yPos, x2, yPos);
        }

        for (int x = qMax(1, xStart); x <= xEnd + 1; x++) {
            int xPos = x * cellWidth;
            int y1 = yStart * cellHeight;
            int y2 = (yEnd + 1) * cellHeight;

            if (x > cellsInLastRow && yEnd == lastRow) {
                if (nRows == 1) {
                    break;
                }
                y2 -= cellWidth;
            }

            painter.drawLine(xPos, y1, xPos, y2);
        }
    }
}
