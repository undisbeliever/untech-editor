#pragma once

#include "gui/controllers/metasprite.h"
#include "gui/widgets/common/ms8aabb.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

class EntityHitboxEditor {
public:
    EntityHitboxEditor(MS::EntityHitboxController& controller);

protected:
    void updateGuiValues();

    void onParameterFinishedEditing();

public:
    Gtk::Grid widget;

private:
    MS::EntityHitboxController& _controller;

    Ms8rectSpinButtons _aabbSpinButtons;
    Gtk::Entry _parameterEntry;

    Gtk::Label _aabbLabel, _aabbCommaLabel, _aabbCrossLabel, _parameterLabel;

    bool _updatingValues;
};
}
}
}
