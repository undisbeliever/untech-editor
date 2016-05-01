#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_ENTITYHITBOXEDITOR_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_ENTITYHITBOXEDITOR_H_

#include "selection.h"
#include "gui/widgets/common/aabb.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

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

    UrectSpinButtons _aabbSpinButtons;
    Gtk::Entry _parameterEntry;

    Gtk::Label _aabbLabel, _aabbCommaLabel, _aabbCrossLabel, _parameterLabel;

    bool _updatingValues;
};
}
}
}

#endif
