#ifndef _UNTECH_GUI_WIDGETS_METASPRITE_FRAMEOBJECTEDITOR_H_
#define _UNTECH_GUI_WIDGETS_METASPRITE_FRAMEOBJECTEDITOR_H_

#include "signals.h"
#include "models/metasprite/frameobject.h"
#include "gui/widgets/common/ms8aabb.h"
#include "gui/widgets/defaults.h"

#include <cassert>
#include <memory>

#include <gtkmm.h>
#include <glibmm/i18n.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

class FrameObjectEditor {
public:
    FrameObjectEditor();

    void setFrameObject(std::shared_ptr<UnTech::MetaSprite::FrameObject> frameObject)
    {
        _frameObject = frameObject;
        updateGuiValues();
    }

protected:
    void updateTileIdRange();
    void updateGuiValues();

public:
    Gtk::Grid widget;

private:
    std::shared_ptr<UnTech::MetaSprite::FrameObject> _frameObject;

    Ms8pointSpinButtons _locationSpinButtons;
    Gtk::SpinButton _tileIdSpinButton;
    Gtk::ComboBoxText _sizeCombo;
    Gtk::SpinButton _orderSpinButton;
    Gtk::CheckButton _hFlipCB, _vFlipCB;

    Gtk::Label _locationLabel, _locationCommaLabel;
    Gtk::Label _tileIdLabel;
    Gtk::Label _sizeLabel;
    Gtk::Label _orderLabel;

    bool _updatingValues;
};
}
}
}

#endif
