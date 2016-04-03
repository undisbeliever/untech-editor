#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_FRAMESETPROPERTIESEDITOR_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_FRAMESETPROPERTIESEDITOR_H_

#include "signals.h"
#include "models/sprite-importer/frameset.h"
#include "gui/widgets/common/aabb.h"
#include "gui/widgets/common/namedlistnameentry.h"
#include "gui/widgets/defaults.h"

#include <gtkmm.h>
#include <glibmm/i18n.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

class FrameSetPropertiesEditor {
public:
    FrameSetPropertiesEditor();

    void setFrameSet(std::shared_ptr<SI::FrameSet> frameSet)
    {
        _frameSet = frameSet;
        updateGuiValues();
    }

    auto signal_selectTransparentClicked()
    {
        return _transparentColorButton.signal_clicked();
    }

protected:
    void updateGuiValues();

public:
    Gtk::Grid widget;

private:
    std::shared_ptr<SI::FrameSet> _frameSet;

    NamedListNameEntry _nameEntry;

    Gtk::Box _imageFilenameBox;
    Gtk::Entry _imageFilenameEntry;
    Gtk::Button _imageFilenameButton;

    Gtk::Box _transparentColorBox;
    Gtk::Entry _transparentColorEntry;
    Gtk::Button _transparentColorButton;

    UsizeSpinButtons _gridFrameSizeSpinButtons;
    UpointSpinButtons _gridOffsetSpinButtons;
    UsizeSpinButtons _gridPaddingSpinButtons;
    UpointSpinButtons _gridOriginSpinButtons;

    Gtk::Label _nameLabel;
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
