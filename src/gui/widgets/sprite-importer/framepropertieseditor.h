#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_FRAMEPROPERTIESEDITOR_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_FRAMEPROPERTIESEDITOR_H_

#include "models/sprite-importer/frame.h"
#include "gui/undo/undostack.h"
#include "gui/widgets/common/aabb.h"
#include "gui/widgets/defaults.h"

#include <gtkmm.h>
#include <glibmm/i18n.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

class FramePropertiesEditor {
public:
    FramePropertiesEditor(Undo::UndoStack& undoStack);

    void setFrame(std::shared_ptr<SI::Frame> frame)
    {
        _frame = frame;
        updateGuiValues();
    }

protected:
    void updateGuiValues();

public:
    Gtk::Grid widget;

private:
    Undo::UndoStack& _undoStack;

    std::shared_ptr<SI::Frame> _frame;

    UpointSpinButtons _gridLocationSpinButtons;
    UrectSpinButtons _locationSpinButtons;
    UpointSpinButtons _originSpinButtons;
    UrectSpinButtons _tileHitboxSpinButtons;

    Glib::RefPtr<Gtk::Adjustment> _spriteOrderAdjustment;
    Gtk::SpinButton _spriteOrderSpinButton;

    Gtk::CheckButton _useGridLocationCB, _useCustomOriginCB, _solidCB;

    Gtk::Alignment _emptySpace;

    Gtk::Label _gridLocationLabel, _gridLocationCommaLabel;
    Gtk::Label _locationLabel, _locationCommaLabel, _locationCrossLabel;
    Gtk::Label _originLabel, _originCommaLabel;
    Gtk::Label _tileHitboxLabel, _tileHitboxCommaLabel, _tileHitboxCrossLabel;
    Gtk::Label _spriteOrderLabel;

    bool _updatingValues;
};
}
}
}

#endif
