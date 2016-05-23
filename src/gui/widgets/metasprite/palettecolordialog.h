#pragma once

#include "../common/colorarea.h"
#include "models/metasprite/palette.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

// This dialog is written under the assumption that this dialog
// is the only way to edit a palette color.

// This dialog will change the color when accept button is pressed.

class PaletteColorDialog : public Gtk::Dialog {
    static const unsigned MAX_COLOR_VALUE = 31;

public:
    PaletteColorDialog(MS::Palette& palette, unsigned colorId, Gtk::Widget& parent);

    void updateGuiValues();

private:
    Gtk::Grid _grid;

    Gtk::Scale _blueScale;
    Gtk::Scale _greenScale;
    Gtk::Scale _redScale;

    ColorArea _colorArea;

    Gtk::Label _blueLabel, _greenLabel, _redLabel;

    MS::Palette& _palette;
    unsigned _colorId;
    Snes::SnesColor _oldColor;
    bool _updatingValues;
};
}
}
}
