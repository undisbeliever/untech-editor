#pragma once

#include "gui/controllers/metasprite.h"
#include "gui/widgets/common/ms8aabb.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

class ActionPointEditor {
public:
    ActionPointEditor(MS::ActionPointController& controller);

protected:
    void updateGuiValues();

    void onParameterFinishedEditing();

public:
    Gtk::Grid widget;

private:
    MS::ActionPointController& _controller;

    Ms8pointSpinButtons _locationSpinButtons;
    Gtk::Entry _parameterEntry;

    Gtk::Label _locationLabel, _locationCommaLabel, _parameterLabel;

    bool _updatingValues;
};
}
}
}
