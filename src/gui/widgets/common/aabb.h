#ifndef _UNTECH_GUI_WIDGETS_AABB_H_
#define _UNTECH_GUI_WIDGETS_AABB_H_

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
    {
        // pass the signal on
        xSpin.signal_value_changed().connect(signal_valueChanged);
        ySpin.signal_value_changed().connect(signal_valueChanged);
    }

    upoint value() const
    {
        assert(xSpin.get_value_as_int() >= 0);
        assert(ySpin.get_value_as_int() >= 0);

        return { (unsigned)xSpin.get_value_as_int(), (unsigned)ySpin.get_value_as_int() };
    }

    void set_value(const upoint& p)
    {
        xSpin.set_value(p.x);
        ySpin.set_value(p.y);
    }

    void set_range(const usize& s)
    {
        xSpin.set_range(0, s.width);
        ySpin.set_range(0, s.height);
    }

    void set_range(const usize& s, unsigned squareSize)
    {
        xSpin.set_range(0, s.width - squareSize);
        ySpin.set_range(0, s.height - squareSize);
    }

    void set_range(unsigned min, const usize& s)
    {
        xSpin.set_range(min, s.width);
        ySpin.set_range(min, s.height);
    }

    Glib::RefPtr<Gtk::Adjustment> xAdjustment, yAdjustment;
    Gtk::SpinButton xSpin, ySpin;

    sigc::signal<void> signal_valueChanged;
};

struct UsizeSpinButtons {

    UsizeSpinButtons()
        : widthAdjustment(Gtk::Adjustment::create(1.0, 0.0, 255.0, 1.0, 4.0, 0.0))
        , heightAdjustment(Gtk::Adjustment::create(1.0, 0.0, 255.0, 1.0, 4.0, 0.0))
        , widthSpin(widthAdjustment)
        , heightSpin(heightAdjustment)
    {
        // pass the signal on
        widthSpin.signal_value_changed().connect(signal_valueChanged);
        heightSpin.signal_value_changed().connect(signal_valueChanged);
    }

    usize value() const
    {
        assert(widthSpin.get_value_as_int() >= 0);
        assert(heightSpin.get_value_as_int() >= 0);

        return { (unsigned)widthSpin.get_value_as_int(), (unsigned)heightSpin.get_value_as_int() };
    }

    void set_value(const usize& s)
    {
        widthSpin.set_value(s.width);
        heightSpin.set_value(s.height);
    }

    void set_range(const usize& s)
    {
        widthSpin.set_range(1, s.width);
        heightSpin.set_range(1, s.height);
    }

    void set_range(const usize& min, const usize& max)
    {
        widthSpin.set_range(min.width, max.width);
        heightSpin.set_range(min.height, max.height);
    }

    Glib::RefPtr<Gtk::Adjustment> widthAdjustment, heightAdjustment;
    Gtk::SpinButton widthSpin, heightSpin;

    sigc::signal<void> signal_valueChanged;
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
        , _range({ 255, 255 })
    {
        // the signal handler will prevent size
        xSpin.signal_value_changed().connect(sigc::mem_fun(this, &UrectSpinButtons::on_valueChanged));
        ySpin.signal_value_changed().connect(sigc::mem_fun(this, &UrectSpinButtons::on_valueChanged));
        widthSpin.signal_value_changed().connect(sigc::mem_fun(this, &UrectSpinButtons::on_valueChanged));
        heightSpin.signal_value_changed().connect(sigc::mem_fun(this, &UrectSpinButtons::on_valueChanged));
    }

    urect value() const
    {
        assert(xSpin.get_value_as_int() >= 0);
        assert(ySpin.get_value_as_int() >= 0);
        assert(widthSpin.get_value_as_int() >= 0);
        assert(heightSpin.get_value_as_int() >= 0);

        return { (unsigned)xSpin.get_value_as_int(), (unsigned)ySpin.get_value_as_int(), (unsigned)widthSpin.get_value_as_int(), (unsigned)heightSpin.get_value_as_int() };
    }

    usize size() const
    {
        assert(widthSpin.get_value_as_int() >= 0);
        assert(heightSpin.get_value_as_int() >= 0);

        return { (unsigned)widthSpin.get_value_as_int(), (unsigned)heightSpin.get_value_as_int() };
    }

    void set_value(const urect& r)
    {
        xSpin.set_value(r.x);
        ySpin.set_value(r.y);
        widthSpin.set_value(r.width);
        heightSpin.set_value(r.height);

        updateRanges();
    }

    void set_range(const usize& s)
    {
        _range = s;
        updateRanges();
    }

    Glib::RefPtr<Gtk::Adjustment> xAdjustment, yAdjustment;
    Glib::RefPtr<Gtk::Adjustment> widthAdjustment, heightAdjustment;
    Gtk::SpinButton xSpin, ySpin;
    Gtk::SpinButton widthSpin, heightSpin;

    sigc::signal<void> signal_valueChanged;

private:
    usize _range;

    inline void updateRanges()
    {
        const urect r = value();

        xSpin.set_range(0, _range.width - r.width);
        ySpin.set_range(0, _range.height - r.height);
        widthSpin.set_range(1, _range.width - r.x);
        heightSpin.set_range(1, _range.height - r.y);
    }

    void on_valueChanged()
    {
        updateRanges();
        signal_valueChanged.emit();
    }
};
}
}

#endif
