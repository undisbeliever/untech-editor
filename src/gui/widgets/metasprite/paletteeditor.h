#pragma once

#include "gui/controllers/metasprite.h"
#include "gui/widgets/common/colortogglebutton.h"
#include "gui/widgets/defaults.h"

#include <glibmm/i18n.h>
#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

class PaletteEditor {
public:
    static const unsigned N_COLORS = 16;

public:
    PaletteEditor(MS::PaletteController& controller);

    void unselectAllColors();

protected:
    void updateGuiValues();

    void on_color_toggled(int cId);

public:
    Gtk::Grid widget;

    // Propagates the selected color to the tile graphic editor.
    // Parameter is negative if no color is selected.
    sigc::signal<void, int> signal_selectColor();

private:
    MS::PaletteController& _controller;

    Gtk::ToggleButton _editSelectOption;

    ColorToggleButton _colorButtons[N_COLORS];

    Gtk::Label _paletteLabel;

    bool _updatingValues;
};
}
}
}
