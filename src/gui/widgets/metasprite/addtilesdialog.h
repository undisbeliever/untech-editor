#pragma once

#include "../common/colorarea.h"
#include "models/metasprite/palette.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

class AddTilesDialog : public Gtk::Dialog {
public:
    AddTilesDialog(MS::FrameSet& frameSet, Gtk::Window& parent);

    void updateGuiValues();

private:
    MS::FrameSet& _frameSet;

    Gtk::Grid _grid;

    Gtk::SpinButton _nNewSmallTilesSpin;
    Gtk::SpinButton _nNewLargeTilesSpin;

    Gtk::Label _titleLabel;
    Gtk::Label _smallLabel, _largeLabel;
    Gtk::Label _smallCount, _largeCount;
};
}
}
}
