/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <sigc++/signal.h>

namespace UnTech {
namespace MetaSprite {
namespace ViewSettings {

class SettingsController;

class Layers {
public:
    Layers(SettingsController& parent);
    Layers(const Layers&) = delete;

    bool frameObjects() const { return _frameObjects; }
    void setFrameObjects(bool);

    bool actionPoints() const { return _actionPoints; }
    void setActionPoints(bool);

    bool entityHitboxes() const { return _entityHitboxes; }
    void setEntityHitboxes(bool);

    bool origin() const { return _origin; }
    void setOrigin(bool);

    bool tileHitbox() const { return _tileHitbox; }
    void setTileHitbox(bool);

    void showAll();

    auto& signal_layersChanged() { return _signal_layersChanged; }

private:
    SettingsController& _parent;

    sigc::signal<void> _signal_layersChanged;

    bool _origin;
    bool _tileHitbox;
    bool _frameObjects;
    bool _actionPoints;
    bool _entityHitboxes;
};

class Zoom {
public:
    enum AspectRatio {
        SQUARE,
        NTSC,
        PAL
    };

    const static unsigned MAX_ZOOM = 15;

public:
    Zoom(SettingsController& parent);
    Zoom(const Zoom&) = delete;

    void setZoom(unsigned zoom);
    void setAspectRatio(AspectRatio ratio);

    unsigned zoom() const { return _zoom; }
    AspectRatio aspectRatio() const { return _aspectRatio; }

    unsigned lineWidth() const { return _lineWidth; }
    double zoomX() const { return _zoomX; }
    double zoomY() const { return _zoomY; }

    auto& signal_zoomChanged() { return _signal_zoomChanged; }

private:
    void updateZoomValues();

private:
    SettingsController& _parent;
    sigc::signal<void> _signal_zoomChanged;

    unsigned _zoom;
    AspectRatio _aspectRatio;

    unsigned _lineWidth;
    double _zoomX, _zoomY;
};

template <typename T>
class BasicSetting {
public:
    using value_type = T;

public:
    BasicSetting(SettingsController& parent);
    BasicSetting(const BasicSetting&) = delete;

    inline const value_type& value() { return _value; }

    void setValue(const value_type& v)
    {
        _value = v;
        _signal_valueChanged.emit();
    }

    auto& signal_valueChanged() { return _signal_valueChanged; }

private:
    SettingsController& _parent;
    sigc::signal<void> _signal_valueChanged;

    value_type _value;
};

class SettingsController {
public:
    SettingsController();
    SettingsController(const SettingsController&) = delete;

    auto& layers() { return _layers; }
    auto& zoom() { return _zoom; }
    auto& selectColorWithMouse() { return _selectColorWithMouse; }

    auto& signal_settingsChanged() { return _signal_settingsChanged; }

private:
    sigc::signal<void> _signal_settingsChanged;

    Layers _layers;
    Zoom _zoom;
    BasicSetting<bool> _selectColorWithMouse;
};

template <typename T>
BasicSetting<T>::BasicSetting(SettingsController& parent)
    : _parent(parent)
{
    _signal_valueChanged.connect(parent.signal_settingsChanged());
}
}
}
}
