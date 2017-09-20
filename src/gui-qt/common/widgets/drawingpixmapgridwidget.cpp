/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "drawingpixmapgridwidget.h"

#include <QMouseEvent>

using namespace UnTech;
using namespace UnTech::GuiQt;

DrawingPixmapGridWidget::DrawingPixmapGridWidget(QWidget* parent)
    : PixmapGridWidget(parent)
    , _lastIndex(-1)
    , _lastPixmapPos()
    , _canDraw(false)
    , _drawing(false)
{
}

void DrawingPixmapGridWidget::setCanDraw(bool canDraw)
{
    if (_canDraw != canDraw) {
        _canDraw = canDraw;

        releaseMouse();

        _lastIndex = -1;
        _lastPixmapPos = QPoint();
        _drawing = false;
    }
}

QPoint DrawingPixmapGridWidget::pixmapPosition(int index, const QPoint& point) const
{
    const QSize& cellSize = this->cellSize();
    const QPixmap& pixmap = pixmaps().at(index);

    int x = point.x() % cellSize.width();
    int y = point.y() % cellSize.height();

    return QPoint(x * pixmap.width() / cellSize.width(),
                  y * pixmap.height() / cellSize.height());
}

void DrawingPixmapGridWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && _canDraw) {
        int index = indexAt(event->pos());

        if (index >= 0) {
            grabMouse(Qt::CrossCursor);

            _drawing = true;
            _lastIndex = index;
            _lastPixmapPos = pixmapPosition(index, event->pos());

            drawPixel(_lastIndex, _lastPixmapPos, true);
        }
    }

    PixmapGridWidget::mousePressEvent(event);
}

void DrawingPixmapGridWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton && _drawing) {
        int index = indexAt(event->pos());

        if (index >= 0) {
            QPoint pixmapPos = pixmapPosition(index, event->pos());

            if (index != _lastIndex || pixmapPos != _lastPixmapPos) {
                _lastIndex = index;
                _lastPixmapPos = pixmapPos;

                drawPixel(index, pixmapPos, false);
            }
        }
    }

    PixmapGridWidget::mouseMoveEvent(event);
}

void DrawingPixmapGridWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && _drawing) {
        releaseMouse();
        _drawing = false;
    }

    PixmapGridWidget::mouseReleaseEvent(event);
}
