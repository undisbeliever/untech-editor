#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_FRAMEOBJECTEDITOR_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_FRAMEOBJECTEDITOR_H_

#include "signals.h"
#include "models/sprite-importer/frameobject.h"
#include "models/sprite-importer/frame.h"
#include "gui/undo/undostack.h"
#include "gui/widgets/common/aabb.h"
#include "gui/widgets/defaults.h"

#include <cassert>
#include <memory>

#include <gtkmm.h>
#include <glibmm/i18n.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

class FrameObjectEditor {
public:
    FrameObjectEditor(Undo::UndoStack& undoStack);

    void setFrameObject(std::shared_ptr<UnTech::SpriteImporter::FrameObject> frameObject)
    {
        _frameObject = frameObject;
        updateGuiValues();
    }

protected:
    void updateGuiValues();

public:
    Gtk::Grid widget;

private:
    Undo::UndoStack& _undoStack;

    std::shared_ptr<UnTech::SpriteImporter::FrameObject> _frameObject;

    UpointSpinButtons _locationSpinButtons;
    Gtk::ComboBoxText _sizeCombo;

    Gtk::Label _locationLabel, _locationCommaLabel, _sizeLabel;

    bool _updatingValues;
};
}
}
}

#endif
