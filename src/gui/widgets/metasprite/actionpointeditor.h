#pragma once

#include "selection.h"
#include "gui/widgets/common/ms8aabb.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

class ActionPointEditor {
public:
    ActionPointEditor(Selection& selection);

protected:
    void updateGuiValues();

    void onParameterFinishedEditing();

public:
    Gtk::Grid widget;

private:
    Selection& _selection;

    Ms8pointSpinButtons _locationSpinButtons;
    Gtk::Entry _parameterEntry;

    Gtk::Label _locationLabel, _locationCommaLabel, _parameterLabel;

    bool _updatingValues;
};
}
}
}
