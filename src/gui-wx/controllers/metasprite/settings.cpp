/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "settings.h"

using namespace UnTech::MetaSprite::ViewSettings;

// SettingsController
// ------------------
SettingsController::SettingsController()
    : _signal_settingsChanged()
    , _layers(*this)
    , _zoom(*this)
    , _selectColorWithMouse(*this)
{
}

// Layers
// ------
Layers::Layers(SettingsController& parent)
    : _parent(parent)
    , _signal_layersChanged()
    , _origin(true)
    , _tileHitbox(true)
    , _frameObjects(true)
    , _actionPoints(true)
    , _entityHitboxes(true)
{
    _signal_layersChanged.connect(_parent.signal_settingsChanged());
}

void Layers::setFrameObjects(bool frameObjects)
{
    _frameObjects = frameObjects;
    _signal_layersChanged.emit();
}

void Layers::setActionPoints(bool actionPoints)
{
    _actionPoints = actionPoints;
    _signal_layersChanged.emit();
}

void Layers::setEntityHitboxes(bool entityHitboxes)
{
    _entityHitboxes = entityHitboxes;
    _signal_layersChanged.emit();
}

void Layers::setOrigin(bool origin)
{
    _origin = origin;
    _signal_layersChanged.emit();
}

void Layers::setTileHitbox(bool tileHitbox)
{
    _tileHitbox = tileHitbox;
    _signal_layersChanged.emit();
}

void Layers::showAll()
{
    _frameObjects = true;
    _actionPoints = true;
    _entityHitboxes = true;
    _origin = true;
    _tileHitbox = true;

    _signal_layersChanged.emit();
}

// Zoom
// ----

Zoom::Zoom(SettingsController& parent)
    : _parent(parent)
    , _signal_zoomChanged()
    , _zoom(3)
    , _aspectRatio(AspectRatio::NTSC)
{
    updateZoomValues();

    _signal_zoomChanged.connect(_parent.signal_settingsChanged());
}

void Zoom::setZoom(unsigned zoom)
{
    if (zoom <= 0) {
        zoom = 1;
    }
    if (zoom > MAX_ZOOM) {
        zoom = MAX_ZOOM;
    }

    _zoom = zoom;
    updateZoomValues();
}

void Zoom::setAspectRatio(AspectRatio ratio)
{
    _aspectRatio = ratio;
    updateZoomValues();
}

void Zoom::updateZoomValues()
{
    // Values taken from bsnes-plus
    const double NTSC_RATIO = 54.0 / 47.0;
    const double PAL_RATIO = 32.0 / 23.0;

    switch (_aspectRatio) {
    case SQUARE:
    default:
        _zoomX = _zoom;
        break;

    case NTSC:
        _zoomX = _zoom * NTSC_RATIO;
        break;

    case PAL:
        _zoomX = _zoom * PAL_RATIO;
        break;
    }

    _zoomY = _zoom;

    _lineWidth = (_zoom / 4) + 1;

    _signal_zoomChanged.emit();
}
