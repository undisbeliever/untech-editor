#pragma once

#include "../common/colorarea.h"
#include "gui/controllers/metasprite.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

class PaletteColorDialog : public Gtk::Dialog {
    static const unsigned MAX_COLOR_VALUE = 31;

public:
    PaletteColorDialog(MS::PaletteController& controller, unsigned colorId, Gtk::Widget& parent);
    virtual ~PaletteColorDialog();

protected:
    void updateGuiValues();

private:
    MS::PaletteController& _controller;

    Gtk::Grid _grid;

    Gtk::Scale _blueScale;
    Gtk::Scale _greenScale;
    Gtk::Scale _redScale;

    ColorArea _colorArea;

    Gtk::Label _blueLabel, _greenLabel, _redLabel;

    unsigned _colorId;
    Snes::SnesColor _color;

    bool _updatingValues;

    // As this dialog edits the model in-place it needs to keep the
    // controller signal connections so it can disconnect them safely
    //
    // ::SHOULDDO check to see if I can reuse this dialog and get away with this::
    sigc::connection _con1;
    sigc::connection _con2;
};
}
}
}
