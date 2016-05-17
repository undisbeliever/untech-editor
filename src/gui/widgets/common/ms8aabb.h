#ifndef _UNTECH_GUI_WIDGETS_AABB_H_
#define _UNTECH_GUI_WIDGETS_AABB_H_

/*
 * A simple set of helper structs to reduce duplicate code holding a
 * ms8point/ms8rect editor.
 *
 * The caller class is responsible for inserting the x/y/with/height
 * SpinBoxes into the contain widget. This is so the objects can be aligned
 * in grids/boxes properly.
 */

#include "models/common/int_ms8_t.h"
#include "models/common/ms8aabb.h"

#include <cassert>
#include <gtkmm/spinbutton.h>
#include <iostream>

namespace UnTech {

namespace Widgets {

struct Ms8pointSpinButtons {

    Ms8pointSpinButtons()
        : xAdjustment(Gtk::Adjustment::create(0.0, int_ms8_t::MIN, int_ms8_t::MAX, 1.0, 4.0, 0.0))
        , yAdjustment(Gtk::Adjustment::create(0.0, int_ms8_t::MIN, int_ms8_t::MAX, 1.0, 4.0, 0.0))
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

    ms8point value() const
    {
        assert(int_ms8_t::isValid(xSpin.get_value_as_int()));
        assert(int_ms8_t::isValid(ySpin.get_value_as_int()));

        return ms8point((unsigned)xSpin.get_value_as_int(), (unsigned)ySpin.get_value_as_int());
    }

    void set_value(const ms8point& p)
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

struct Ms8rectSpinButtons {

    Ms8rectSpinButtons()
        : xAdjustment(Gtk::Adjustment::create(0.0, int_ms8_t::MIN, int_ms8_t::MAX, 1.0, 4.0, 0.0))
        , yAdjustment(Gtk::Adjustment::create(0.0, int_ms8_t::MIN, int_ms8_t::MAX, 1.0, 4.0, 0.0))
        , widthAdjustment(Gtk::Adjustment::create(1.0, 1.0, 255.0, 1.0, 4.0, 0.0))
        , heightAdjustment(Gtk::Adjustment::create(1.0, 1.0, 255.0, 1.0, 4.0, 0.0))
        , xSpin(xAdjustment)
        , ySpin(yAdjustment)
        , widthSpin(widthAdjustment)
        , heightSpin(heightAdjustment)
        , _updating(false)
    {
        // the signal handler will prevent size
        xSpin.signal_value_changed().connect(sigc::mem_fun(this, &Ms8rectSpinButtons::on_valueChanged));
        xSpin.signal_focus_out_event().connect(signal_focus_out_event);

        ySpin.signal_value_changed().connect(sigc::mem_fun(this, &Ms8rectSpinButtons::on_valueChanged));
        ySpin.signal_focus_out_event().connect(signal_focus_out_event);

        widthSpin.signal_value_changed().connect(sigc::mem_fun(this, &Ms8rectSpinButtons::on_valueChanged));
        widthSpin.signal_focus_out_event().connect(signal_focus_out_event);

        heightSpin.signal_value_changed().connect(sigc::mem_fun(this, &Ms8rectSpinButtons::on_valueChanged));
        heightSpin.signal_focus_out_event().connect(signal_focus_out_event);
    }

    ms8rect value() const
    {
        assert(int_ms8_t::isValid(xSpin.get_value_as_int()));
        assert(int_ms8_t::isValid(ySpin.get_value_as_int()));
        assert(widthSpin.get_value_as_int() >= 1);
        assert(heightSpin.get_value_as_int() >= 1);

        return ms8rect(xSpin.get_value_as_int(), ySpin.get_value_as_int(),
                       (unsigned)widthSpin.get_value_as_int(), (unsigned)heightSpin.get_value_as_int());
    }

    void set_value(const ms8rect& r)
    {
        const auto oldValue = value();

        _updating = true;

        xSpin.set_value(r.x);
        ySpin.set_value(r.y);
        widthSpin.set_value(r.width);
        heightSpin.set_value(r.height);

        _updating = false;

        if (oldValue != value()) {
            signal_valueChanged.emit();
        }
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
    void on_valueChanged()
    {
        if (!_updating) {
            signal_valueChanged.emit();
        }
    }

    bool _updating;
};
}
}

#endif
