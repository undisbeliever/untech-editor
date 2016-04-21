#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_ACTIONPOINTEDITOR_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_ACTIONPOINTEDITOR_H_

#include "signals.h"
#include "models/sprite-importer/actionpoint.h"
#include "models/sprite-importer/frame.h"
#include "models/common/string.h"
#include "gui/widgets/common/aabb.h"
#include "gui/widgets/defaults.h"

#include <cassert>
#include <memory>

#include <gtkmm.h>
#include <glibmm/i18n.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

class ActionPointEditor {
public:
    ActionPointEditor();

    void setActionPoint(UnTech::SpriteImporter::ActionPoint* actionPoint)
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
    UnTech::SpriteImporter::ActionPoint* _actionPoint;

    UpointSpinButtons _locationSpinButtons;
    Gtk::Entry _parameterEntry;

    Gtk::Label _locationLabel, _locationCommaLabel, _parameterLabel;

    bool _updatingValues;
};
}
}
}

#endif
