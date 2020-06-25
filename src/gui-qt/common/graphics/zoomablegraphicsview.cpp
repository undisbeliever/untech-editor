/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "zoomablegraphicsview.h"
#include "zoomsettings.h"

#include <QScrollBar>
#include <QWheelEvent>

using namespace UnTech::GuiQt;

ZoomableGraphicsView::ZoomableGraphicsView(QWidget* parent)
    : QGraphicsView(parent)
    , _zoomSettings(nullptr)
    , _enableZoomWithMouseWheel(true)
{
}

ZoomableGraphicsView::ZoomableGraphicsView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
    , _zoomSettings(nullptr)
    , _enableZoomWithMouseWheel(true)
{
}

void ZoomableGraphicsView::setEnableZoomWithMouseWheel(bool enableZoomWithMouseWheel)
{
    _enableZoomWithMouseWheel = enableZoomWithMouseWheel;
}

void ZoomableGraphicsView::setZoomSettings(ZoomSettings* zoomSettings)
{
    if (_zoomSettings == zoomSettings) {
        return;
    }

    if (_zoomSettings) {
        _zoomSettings->disconnect(this);
    }
    _zoomSettings = zoomSettings;

    if (_zoomSettings) {
        onZoomSettingsChanged();
        connect(_zoomSettings, &ZoomSettings::transformChanged,
                this, &ZoomableGraphicsView::onZoomSettingsChanged);
    }
    else {
        resetTransform();
    }
}

void ZoomableGraphicsView::wheelEvent(QWheelEvent* event)
{
    if (_zoomSettings
        && _enableZoomWithMouseWheel
        && event->modifiers() & Qt::ControlModifier) {

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        const QPoint mousePos = event->position().toPoint();
#else
        const QPoint mousePos = event->pos();
#endif

        const qreal oldZoom = _zoomSettings->zoom();
        const QPointF oldScenePos = mapToScene(mousePos);

        if (event->angleDelta().y() > 0) {
            _zoomSettings->zoomIn();
        }
        else {
            _zoomSettings->zoomOut();
        }

        if (_zoomSettings->zoom() != oldZoom) {
            const QPoint delta = mousePos - mapFromScene(oldScenePos);

            horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
            verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        }
    }
    else {
        QGraphicsView::wheelEvent(event);
    }
}

void ZoomableGraphicsView::onZoomSettingsChanged()
{
    setTransform(_zoomSettings->transform());
}
