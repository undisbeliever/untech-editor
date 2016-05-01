#ifndef _UNTECH_GUI_WIDGETS_METASPRITE_ENTITYHITBOXEDITOR_H_
#define _UNTECH_GUI_WIDGETS_METASPRITE_ENTITYHITBOXEDITOR_H_

#include "selection.h"
#include "gui/widgets/common/ms8aabb.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

class EntityHitboxEditor {
public:
    EntityHitboxEditor(Selection& selection);

protected:
    void updateGuiValues();

    void onParameterFinishedEditing();

public:
    Gtk::Grid widget;

private:
    Selection& _selection;

    Ms8rectSpinButtons _aabbSpinButtons;
    Gtk::Entry _parameterEntry;

    Gtk::Label _aabbLabel, _aabbCommaLabel, _aabbCrossLabel, _parameterLabel;

    bool _updatingValues;
};
}
}
}

#endif
