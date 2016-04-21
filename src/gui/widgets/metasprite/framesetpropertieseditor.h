#ifndef _UNTECH_GUI_WIDGETS_METASPRITE_FRAMESETPROPERTIESEDITOR_H_
#define _UNTECH_GUI_WIDGETS_METASPRITE_FRAMESETPROPERTIESEDITOR_H_

#include "signals.h"
#include "models/metasprite/frameset.h"
#include "gui/widgets/common/ms8aabb.h"
#include "gui/widgets/common/namedlistnameentry.h"
#include "gui/widgets/defaults.h"

#include <gtkmm.h>
#include <glibmm/i18n.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

class FrameSetPropertiesEditor {
public:
    FrameSetPropertiesEditor();

    void setFrameSet(MS::FrameSet* frameSet)
    {
        _frameSet = frameSet;
        updateGuiValues();
    }

protected:
    void updateGuiValues();

public:
    Gtk::Grid widget;

private:
    MS::FrameSet* _frameSet;

    NamedListNameEntry _nameEntry;

    Gtk::Label _nameLabel;

    bool _updatingValues;
};
}
}
}

#endif
