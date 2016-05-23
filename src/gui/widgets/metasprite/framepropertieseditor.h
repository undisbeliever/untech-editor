#pragma once

#include "selection.h"
#include "gui/widgets/common/ms8aabb.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

class FramePropertiesEditor {
public:
    FramePropertiesEditor(Selection& selection);

protected:
    void updateGuiValues();

public:
    Gtk::Grid widget;

private:
    Selection& _selection;

    Ms8rectSpinButtons _tileHitboxSpinButtons;

    Gtk::CheckButton _solidCB;

    Gtk::Alignment _emptySpace;

    Gtk::Label _tileHitboxLabel, _tileHitboxCommaLabel, _tileHitboxCrossLabel;
    Gtk::Label _spriteOrderLabel;

    bool _updatingValues;
};
}
}
}
