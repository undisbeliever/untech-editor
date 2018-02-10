/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "zoomablegraphicsview.h"
#include "zoomsettings.h"

using namespace UnTech::GuiQt;

ZoomableGraphicsView::ZoomableGraphicsView(QWidget* parent)
    : QGraphicsView(parent)
    , _zoomSettings(nullptr)
{
}

ZoomableGraphicsView::ZoomableGraphicsView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
    , _zoomSettings(nullptr)
{
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

void ZoomableGraphicsView::onZoomSettingsChanged()
{
    setTransform(_zoomSettings->transform());
}