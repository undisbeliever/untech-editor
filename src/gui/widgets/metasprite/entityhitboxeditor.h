#ifndef _UNTECH_GUI_WIDGETS_METASPRITE_ENTITYHITBOXEDITOR_H_
#define _UNTECH_GUI_WIDGETS_METASPRITE_ENTITYHITBOXEDITOR_H_

#include "signals.h"
#include "models/metasprite/entityhitbox.h"
#include "models/common/string.h"
#include "gui/widgets/common/ms8aabb.h"
#include "gui/widgets/defaults.h"

#include <cassert>
#include <memory>

#include <gtkmm.h>
#include <glibmm/i18n.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

class EntityHitboxEditor {
public:
    EntityHitboxEditor();

    void setEntityHitbox(UnTech::MetaSprite::EntityHitbox* entityHitbox)
    {
        _entityHitbox = entityHitbox;
        updateGuiValues();
    }

protected:
    void updateGuiValues();

    void onParameterFinishedEditing();

public:
    Gtk::Grid widget;

private:
    UnTech::MetaSprite::EntityHitbox* _entityHitbox;

    Ms8rectSpinButtons _aabbSpinButtons;
    Gtk::Entry _parameterEntry;

    Gtk::Label _aabbLabel, _aabbCommaLabel, _aabbCrossLabel, _parameterLabel;

    bool _updatingValues;
};
}
}
}

#endif
