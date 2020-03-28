/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
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
    , _selected(-1)
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

void PixmapGridWidget::setPixmap(int index, const QPixmap& pixmap)
{
    if (index >= 0 && index < _pixmaps.size()) {
        _pixmaps.replace(index, pixmap);

        int nColumns = columnCount();
        int x = index % nColumns;
        int y = index / nColumns;

        update(x * _cellSize.width(), y * _cellSize.height(),
               _cellSize.width(), _cellSize.height());
    }
}

void PixmapGridWidget::setSelected(int selected)
{
    if (_selected != selected) {
        const int oldSelected = _selected;

        _selected = selected;

        int nColumns = columnCount();
        auto updateCell = [&](int index) {
            if (index >= 0 || index < _pixmaps.size()) {
                int x = index % nColumns;
                int y = index / nColumns;

                // must include the selected outline in the update
                update(x * _cellSize.width() - 1, y * _cellSize.height() - 1,
                       _cellSize.width() + 2, _cellSize.height() + 2);
            }
        };
        updateCell(oldSelected);
        updateCell(_selected);
    }
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

int PixmapGridWidget::columnCount() const
{
    return qMax(1, (width() - 1) / _cellSize.width());
}

void PixmapGridWidget::updateWindowSize()
{
    setMinimumSize(_cellSize + QSize(1, 1));

    // required for resize() to work when this widget is inside a Layout
    int n = qMax(1, _pixmaps.size());
    setMaximumSize(_cellSize * n + QSize(1, 1));

    resize(width(), heightForWidth(width()));
}

int PixmapGridWidget::indexAt(const QPoint& point) const
{
    if (point.x() < 0 || point.y() < 0) {
        return -1;
    }

    const int nColumns = columnCount();
    int x = point.x() / _cellSize.width();
    int y = point.y() / _cellSize.height();

    if (x >= nColumns) {
        return -1;
    }

    int i = y * nColumns + x;
    if (i < _pixmaps.size()) {
        return i;
    }
    else {
        return -1;
    }
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

    // Draw Selected Outline
    if (_selected >= 0 && _selected < _pixmaps.size()) {
        static const QPen white(Qt::white, 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
        static const QPen black(Qt::black, 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);

        QRectF r(_selected % nColumns * cellWidth, _selected / nColumns * cellHeight,
                 cellWidth, cellHeight);

        painter.setBrush(Qt::NoBrush);

        painter.setPen(black);
        painter.drawRect(r);

        r.adjust(1, 1, -1, -1);
        painter.setPen(white);
        painter.drawRect(r);
    }
}

void PixmapGridWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        int index = indexAt(event->pos());

        if (index >= 0) {
            emit cellClicked(index);
        }
    }

    QWidget::mousePressEvent(event);
}
