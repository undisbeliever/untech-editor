#ifndef _UNTECH_GUI_WIDGETS_METASPRITE_FRAMEPROPERTIESEDITOR_H_
#define _UNTECH_GUI_WIDGETS_METASPRITE_FRAMEPROPERTIESEDITOR_H_

#include "models/metasprite/frame.h"
#include "gui/widgets/common/ms8aabb.h"
#include "gui/widgets/defaults.h"

#include <gtkmm.h>
#include <glibmm/i18n.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

class FramePropertiesEditor {
public:
    FramePropertiesEditor();

    void setFrame(std::shared_ptr<MS::Frame> frame)
    {
        _frame = frame;
        updateGuiValues();
    }

protected:
    void updateGuiValues();

public:
    Gtk::Grid widget;

private:
    std::shared_ptr<MS::Frame> _frame;

    Ms8rectSpinButtons _tileHitboxSpinButtons;

    Gtk::CheckButton _solidCB;

    Gtk::Alignment _emptySpace;

    Gtk::Label _tileHitboxLabel, _tileHitboxCommaLabel, _tileHitboxCrossLabel;
    Gtk::Label _spriteOrderLabel;

    bool _updatingValues;
};
}
}
}

#endif
