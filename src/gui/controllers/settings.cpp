#include "settings.h"

using namespace UnTech::Controller;

Settings::Settings()
    : _zoom(3)
    , _aspectRatio(NTSC)
{
    updateZoomValues();
}

void Settings::updateZoomValues()
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

    _signal_zoomChanged.emit();
}

void Settings::setZoom(int zoom)
{
    if (zoom <= 0) {
        zoom = 1;
    }
    if (zoom > 15) {
        zoom = 15;
    }

    if (_zoom != zoom) {
        _zoom = zoom;
        updateZoomValues();
    }
}

void Settings::setAspectRatio(AspectRatio ratio)
{
    if (_aspectRatio != ratio) {
        _aspectRatio = ratio;
        updateZoomValues();
    }
}
