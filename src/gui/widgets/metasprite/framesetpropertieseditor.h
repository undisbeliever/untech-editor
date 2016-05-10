#ifndef _UNTECH_GUI_WIDGETS_METASPRITE_FRAMESETPROPERTIESEDITOR_H_
#define _UNTECH_GUI_WIDGETS_METASPRITE_FRAMESETPROPERTIESEDITOR_H_

#include "selection.h"
#include "gui/widgets/common/enumclasscombobox.h"
#include "gui/widgets/common/ms8aabb.h"
#include "gui/widgets/common/namedlistnameentry.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;
namespace MSF = UnTech::MetaSpriteFormat;

class FrameSetPropertiesEditor {
public:
    FrameSetPropertiesEditor(Selection& selection);

protected:
    void updateGuiValues();

public:
    Gtk::Grid widget;

private:
    Selection& _selection;

    NamedListNameEntry _nameEntry;
    EnumClassComboBox<MSF::TilesetType> _tilesetTypeCombo;

    Gtk::Label _nameLabel;
    Gtk::Label _tilesetTypeLabel;

    bool _updatingValues;
};
}
}
}

#endif
