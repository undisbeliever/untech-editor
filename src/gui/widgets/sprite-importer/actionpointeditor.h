#pragma once

#include "gui/controllers/sprite-importer.h"
#include "gui/widgets/common/aabb.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

class ActionPointEditor {
public:
    ActionPointEditor(SI::SpriteImporterController& controller);

protected:
    void updateGuiValues();
    void updateRange();

    void onParameterFinishedEditing();

public:
    Gtk::Grid widget;

private:
    SI::SpriteImporterController& _controller;

    UpointSpinButtons _locationSpinButtons;
    Gtk::Entry _parameterEntry;

    Gtk::Label _locationLabel, _locationCommaLabel, _parameterLabel;

    bool _updatingValues;
};
}
}
}
