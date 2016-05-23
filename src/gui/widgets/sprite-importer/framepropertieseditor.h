#pragma once

#include "selection.h"
#include "gui/widgets/common/aabb.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

class FramePropertiesEditor {
public:
    FramePropertiesEditor(Selection& selection);

protected:
    void updateGuiValues();

public:
    Gtk::Grid widget;

private:
    Selection& _selection;

    UpointSpinButtons _gridLocationSpinButtons;
    UrectSpinButtons _locationSpinButtons;
    UpointSpinButtons _originSpinButtons;
    UrectSpinButtons _tileHitboxSpinButtons;

    Glib::RefPtr<Gtk::Adjustment> _spriteOrderAdjustment;
    Gtk::SpinButton _spriteOrderSpinButton;

    Gtk::CheckButton _useGridLocationCB, _useCustomOriginCB, _solidCB;

    Gtk::Alignment _emptySpace;

    Gtk::Label _gridLocationLabel, _gridLocationCommaLabel;
    Gtk::Label _locationLabel, _locationCommaLabel, _locationCrossLabel;
    Gtk::Label _originLabel, _originCommaLabel;
    Gtk::Label _tileHitboxLabel, _tileHitboxCommaLabel, _tileHitboxCrossLabel;
    Gtk::Label _spriteOrderLabel;

    bool _updatingValues;
};
}
}
}
