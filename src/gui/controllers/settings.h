#pragma once

#include <sigc++/signal.h>

namespace UnTech {
namespace Controller {

class Settings {
public:
    enum AspectRatio {
        SQUARE,
        NTSC,
        PAL
    };

public:
    Settings();
    ~Settings() = default;

    void setZoom(int);
    void setAspectRatio(AspectRatio);

    int zoom() const { return _zoom; }
    AspectRatio aspectRatio() const { return _aspectRatio; }
    int lineWidth() const { return _lineWidth; }
    double zoomX() const { return _zoomX; }
    double zoomY() const { return _zoomY; }

    auto& signal_zoomChanged() { return _signal_zoomChanged; }

private:
    void updateZoomValues();

private:
    // this is an int because of the way gtkmm menu's work
    int _zoom;
    AspectRatio _aspectRatio;
    int _lineWidth;
    double _zoomX, _zoomY;

    sigc::signal<void> _signal_zoomChanged;
};
}
}
