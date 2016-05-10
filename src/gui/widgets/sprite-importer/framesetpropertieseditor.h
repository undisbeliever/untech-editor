#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_FRAMESETPROPERTIESEDITOR_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_FRAMESETPROPERTIESEDITOR_H_

#include "selection.h"
#include "gui/widgets/common/aabb.h"
#include "gui/widgets/common/colortogglebutton.h"
#include "gui/widgets/common/enumclasscombobox.h"
#include "gui/widgets/common/namedlistnameentry.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;
namespace MSF = UnTech::MetaSpriteFormat;

class FrameSetPropertiesEditor {
public:
    FrameSetPropertiesEditor(Selection& selection);

protected:
    void updateGuiValues();

    void on_imageFilenameButtonClicked();

public:
    Gtk::Grid widget;

private:
    Selection& _selection;

    NamedListNameEntry _nameEntry;
    EnumClassComboBox<MSF::TilesetType> _tilesetTypeCombo;

    Gtk::Box _imageFilenameBox;
    Gtk::Entry _imageFilenameEntry;
    Gtk::Button _imageFilenameButton;

    Gtk::Box _transparentColorBox;
    Gtk::Entry _transparentColorEntry;
    ColorToggleButton _transparentColorButton;

    UsizeSpinButtons _gridFrameSizeSpinButtons;
    UpointSpinButtons _gridOffsetSpinButtons;
    UsizeSpinButtons _gridPaddingSpinButtons;
    UpointSpinButtons _gridOriginSpinButtons;

    Gtk::Label _nameLabel;
    Gtk::Label _tilesetTypeLabel;
    Gtk::Label _imageFilenameLabel;
    Gtk::Label _transparentColorLabel;
    Gtk::Label _gridFrameSizeLabel, _gridFrameSizeCrossLabel;
    Gtk::Label _gridOffsetLabel, _gridOffsetCommaLabel;
    Gtk::Label _gridPaddingLabel, _gridPaddingCrossLabel;
    Gtk::Label _gridOriginLabel, _gridOriginCommaLabel;

    bool _updatingValues;
};
}
}
}

#endif
