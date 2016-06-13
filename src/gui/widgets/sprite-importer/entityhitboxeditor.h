#pragma once

#include "gui/controllers/sprite-importer.h"
#include "gui/widgets/common/aabb.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

class EntityHitboxEditor {
public:
    EntityHitboxEditor(SI::SpriteImporterController& controller);

protected:
    void updateGuiValues();
    void updateRange();

    void onParameterFinishedEditing();

public:
    Gtk::Grid widget;

private:
    SI::SpriteImporterController& _controller;

    UrectSpinButtons _aabbSpinButtons;
    Gtk::Entry _parameterEntry;

    Gtk::Label _aabbLabel, _aabbCommaLabel, _aabbCrossLabel, _parameterLabel;

    bool _updatingValues;
};
}
}
}
