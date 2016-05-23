#pragma once

/*
 * A simple set of helper structs to reduce duplicate code holding a
 * upoint/usize/urect editor.
 *
 * The caller class is responsible for inserting the x/y/with/height
 * SpinBoxes into the contain widget. This is so the objects can be aligned
 * in grids/boxes properly.
 */

#include "models/common/aabb.h"

#include <cassert>
#include <gtkmm/spinbutton.h>
#include <iostream>

namespace UnTech {

namespace Widgets {

struct UpointSpinButtons {

    UpointSpinButtons()
        : xAdjustment(Gtk::Adjustment::create(0.0, 0.0, 255.0, 1.0, 4.0, 0.0))
        , yAdjustment(Gtk::Adjustment::create(0.0, 0.0, 255.0, 1.0, 4.0, 0.0))
        , xSpin(xAdjustment)
        , ySpin(yAdjustment)
        , _updating(false)
    {
        // pass the signal on if not updating
        xSpin.signal_value_changed().connect([this](void) {
            if (!_updating) {
                signal_valueChanged.emit();
            }
        });
        xSpin.signal_focus_out_event().connect(signal_focus_out_event);

        ySpin.signal_value_changed().connect([this](void) {
            if (!_updating) {
                signal_valueChanged.emit();
            }
        });
        ySpin.signal_focus_out_event().connect(signal_focus_out_event);
    }

    upoint value() const
    {
        assert(xSpin.get_value_as_int() >= 0);
        assert(ySpin.get_value_as_int() >= 0);

        return upoint((unsigned)xSpin.get_value_as_int(), (unsigned)ySpin.get_value_as_int());
    }

    void set_value(const upoint& p)
    {
        const auto oldValue = value();

        _updating = true;

        xSpin.set_value(p.x);
        ySpin.set_value(p.y);

        _updating = false;

        if (oldValue != value()) {
            signal_valueChanged.emit();
        }
    }

    void set_range(const usize& s)
    {
        const auto oldValue = value();

        _updating = true;

        xSpin.set_range(0, s.width);
        ySpin.set_range(0, s.height);

        _updating = false;

        if (oldValue != value()) {
            signal_valueChanged.emit();
        }
    }

    void set_range(const usize& s, unsigned squareSize)
    {
        const auto oldValue = value();

        _updating = true;

        xSpin.set_range(0, s.width - squareSize);
        ySpin.set_range(0, s.height - squareSize);

        _updating = false;

        if (oldValue != value()) {
            signal_valueChanged.emit();
        }
    }

    void set_range(unsigned min, const usize& s)
    {
        const auto oldValue = value();

        _updating = true;

        xSpin.set_range(min, s.width);
        ySpin.set_range(min, s.height);

        _updating = false;

        if (oldValue != value()) {
            signal_valueChanged.emit();
        }
    }

    void set_sensitive(bool s)
    {
        xSpin.set_sensitive(s);
        ySpin.set_sensitive(s);
    }

    Glib::RefPtr<Gtk::Adjustment> xAdjustment, yAdjustment;
    Gtk::SpinButton xSpin, ySpin;

    sigc::signal<void> signal_valueChanged;
    sigc::signal<bool, GdkEventFocus*> signal_focus_out_event;

private:
    bool _updating;
};

struct UsizeSpinButtons {

    UsizeSpinButtons()
        : widthAdjustment(Gtk::Adjustment::create(1.0, 0.0, 255.0, 1.0, 4.0, 0.0))
        , heightAdjustment(Gtk::Adjustment::create(1.0, 0.0, 255.0, 1.0, 4.0, 0.0))
        , widthSpin(widthAdjustment)
        , heightSpin(heightAdjustment)
        , _updating(false)
    {
        // pass the signal on if not updating
        widthSpin.signal_value_changed().connect([this](void) {
            if (!_updating) {
                signal_valueChanged.emit();
            }
        });
        widthSpin.signal_focus_out_event().connect(signal_focus_out_event);

        heightSpin.signal_value_changed().connect([this](void) {
            if (!_updating) {
                signal_valueChanged.emit();
            }
        });
        heightSpin.signal_focus_out_event().connect(signal_focus_out_event);
    }

    usize value() const
    {
        assert(widthSpin.get_value_as_int() >= 0);
        assert(heightSpin.get_value_as_int() >= 0);

        return usize((unsigned)widthSpin.get_value_as_int(), (unsigned)heightSpin.get_value_as_int());
    }

    void set_value(const usize& s)
    {
        const auto oldValue = value();

        _updating = true;

        widthSpin.set_value(s.width);
        heightSpin.set_value(s.height);

        _updating = false;

        if (oldValue != value()) {
            signal_valueChanged.emit();
        }
    }

    void set_range(const usize& s)
    {
        const auto oldValue = value();

        _updating = true;

        widthSpin.set_range(1, s.width);
        heightSpin.set_range(1, s.height);

        _updating = false;

        if (oldValue != value()) {
            signal_valueChanged.emit();
        }
    }

    void set_range(const usize& min, const usize& max)
    {
        const auto oldValue = value();

        _updating = true;

        widthSpin.set_range(min.width, max.width);
        heightSpin.set_range(min.height, max.height);

        _updating = false;

        if (oldValue != value()) {
            signal_valueChanged.emit();
        }
    }

    void set_sensitive(bool s)
    {
        widthSpin.set_sensitive(s);
        heightSpin.set_sensitive(s);
    }

    Glib::RefPtr<Gtk::Adjustment> widthAdjustment, heightAdjustment;
    Gtk::SpinButton widthSpin, heightSpin;

    sigc::signal<void> signal_valueChanged;
    sigc::signal<bool, GdkEventFocus*> signal_focus_out_event;

private:
    bool _updating;
};

struct UrectSpinButtons {

    UrectSpinButtons()
        : xAdjustment(Gtk::Adjustment::create(0.0, 0.0, 255.0, 1.0, 4.0, 0.0))
        , yAdjustment(Gtk::Adjustment::create(0.0, 0.0, 255.0, 1.0, 4.0, 0.0))
        , widthAdjustment(Gtk::Adjustment::create(1.0, 0.0, 255.0, 1.0, 4.0, 0.0))
        , heightAdjustment(Gtk::Adjustment::create(1.0, 0.0, 255.0, 1.0, 4.0, 0.0))
        , xSpin(xAdjustment)
        , ySpin(yAdjustment)
        , widthSpin(widthAdjustment)
        , heightSpin(heightAdjustment)
        , _range(255, 255)
        , _minSize(1, 1)
        , _maxSize({ 255, 255 })
        , _updating(false)
    {
        // the signal handler will prevent size
        xSpin.signal_value_changed().connect(sigc::mem_fun(this, &UrectSpinButtons::on_valueChanged));
        xSpin.signal_focus_out_event().connect(signal_focus_out_event);

        ySpin.signal_value_changed().connect(sigc::mem_fun(this, &UrectSpinButtons::on_valueChanged));
        ySpin.signal_focus_out_event().connect(signal_focus_out_event);

        widthSpin.signal_value_changed().connect(sigc::mem_fun(this, &UrectSpinButtons::on_valueChanged));
        widthSpin.signal_focus_out_event().connect(signal_focus_out_event);

        heightSpin.signal_value_changed().connect(sigc::mem_fun(this, &UrectSpinButtons::on_valueChanged));
        heightSpin.signal_focus_out_event().connect(signal_focus_out_event);
    }

    urect value() const
    {
        assert(xSpin.get_value_as_int() >= 0);
        assert(ySpin.get_value_as_int() >= 0);
        assert(widthSpin.get_value_as_int() >= 0);
        assert(heightSpin.get_value_as_int() >= 0);

        return urect((unsigned)xSpin.get_value_as_int(), (unsigned)ySpin.get_value_as_int(),
                     (unsigned)widthSpin.get_value_as_int(), (unsigned)heightSpin.get_value_as_int());
    }

    usize size() const
    {
        assert(widthSpin.get_value_as_int() >= 0);
        assert(heightSpin.get_value_as_int() >= 0);

        return usize((unsigned)widthSpin.get_value_as_int(), (unsigned)heightSpin.get_value_as_int());
    }

    void set_value(const urect& r)
    {
        const auto oldValue = value();

        _updating = true;

        xSpin.set_value(r.x);
        ySpin.set_value(r.y);
        widthSpin.set_value(r.width);
        heightSpin.set_value(r.height);

        updateRangesNoSignal();

        _updating = false;

        if (oldValue != value()) {
            signal_valueChanged.emit();
        }
    }

    void set_range(const usize& s)
    {
        _range = s;
        updateRanges();
    }

    void set_minSize(const usize& s)
    {
        _minSize = s;
        updateRanges();
    }

    void set_maxSize(const usize& s)
    {
        _maxSize = s;
        updateRanges();
    }

    void set_sensitive(bool s)
    {
        xSpin.set_sensitive(s);
        ySpin.set_sensitive(s);
        widthSpin.set_sensitive(s);
        heightSpin.set_sensitive(s);
    }

    Glib::RefPtr<Gtk::Adjustment> xAdjustment, yAdjustment;
    Glib::RefPtr<Gtk::Adjustment> widthAdjustment, heightAdjustment;
    Gtk::SpinButton xSpin, ySpin;
    Gtk::SpinButton widthSpin, heightSpin;

    sigc::signal<void> signal_valueChanged;
    sigc::signal<bool, GdkEventFocus*> signal_focus_out_event;

private:
    usize _range, _minSize, _maxSize;

    inline void updateRanges()
    {
        const urect oldValue = value();

        _updating = true;

        updateRangesNoSignal();

        _updating = false;

        if (oldValue != value()) {
            signal_valueChanged.emit();
        }
    }

    inline void updateRangesNoSignal()
    {
        const urect r = value();

        xSpin.set_range(0, _range.width - r.width);
        ySpin.set_range(0, _range.height - r.height);
        widthSpin.set_range(_minSize.width, std::min(_maxSize.width, _range.width - r.x));
        heightSpin.set_range(_minSize.height, std::min(_maxSize.height, _range.height - r.y));
    }

    void on_valueChanged()
    {
        updateRangesNoSignal();
        signal_valueChanged.emit();
    }

private:
    bool _updating;
};
}
}
