#pragma once

#include "selection.h"
#include "gui/widgets/common/ms8aabb.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

class FrameObjectEditor {
public:
    FrameObjectEditor(Selection& selection);

protected:
    void updateTileIdRange();
    void updateGuiValues();

public:
    Gtk::Grid widget;

private:
    Selection& _selection;

    Ms8pointSpinButtons _locationSpinButtons;
    Gtk::SpinButton _tileIdSpinButton;
    Gtk::ComboBoxText _sizeCombo;
    Gtk::SpinButton _orderSpinButton;
    Gtk::CheckButton _hFlipCB, _vFlipCB;

    Gtk::Label _locationLabel, _locationCommaLabel;
    Gtk::Label _tileIdLabel;
    Gtk::Label _sizeLabel;
    Gtk::Label _orderLabel;

    bool _updatingValues;
};
}
}
}
