#pragma once

#include "selection.h"
#include "gui/widgets/common/aabb.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

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

    UpointSpinButtons _locationSpinButtons;
    Gtk::Entry _parameterEntry;

    Gtk::Label _locationLabel, _locationCommaLabel, _parameterLabel;

    bool _updatingValues;
};
}
}
}
