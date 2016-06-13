#pragma once

#include "gui/controllers/sprite-importer.h"
#include "gui/widgets/common/aabb.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

class FrameObjectEditor {
public:
    FrameObjectEditor(SI::SpriteImporterController& controller);

protected:
    void updateGuiValues();
    void updateRange();

public:
    Gtk::Grid widget;

private:
    SI::SpriteImporterController& _controller;

    UpointSpinButtons _locationSpinButtons;
    Gtk::ComboBoxText _sizeCombo;

    Gtk::Label _locationLabel, _locationCommaLabel, _sizeLabel;

    bool _updatingValues;
};
}
}
}
