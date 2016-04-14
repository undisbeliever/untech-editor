#ifndef _UNTECH_GUI_WIDGETS_METASPRITE_ACTIONPOINTEDITOR_H_
#define _UNTECH_GUI_WIDGETS_METASPRITE_ACTIONPOINTEDITOR_H_

#include "signals.h"
#include "models/metasprite/actionpoint.h"
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

class ActionPointEditor {
public:
    ActionPointEditor();

    void setActionPoint(std::shared_ptr<UnTech::MetaSprite::ActionPoint> actionPoint)
    {
        _actionPoint = actionPoint;
        updateGuiValues();
    }

protected:
    void updateGuiValues();

    void onParameterFinishedEditing();

public:
    Gtk::Grid widget;

private:
    std::shared_ptr<UnTech::MetaSprite::ActionPoint> _actionPoint;

    Ms8pointSpinButtons _locationSpinButtons;
    Gtk::Entry _parameterEntry;

    Gtk::Label _locationLabel, _locationCommaLabel, _parameterLabel;

    bool _updatingValues;
};
}
}
}

#endif
