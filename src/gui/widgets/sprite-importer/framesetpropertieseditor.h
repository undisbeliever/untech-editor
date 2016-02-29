#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_FRAMESETPROPERTIESEDITOR_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_FRAMESETPROPERTIESEDITOR_H_

#include "signals.h"
#include "models/sprite-importer/frameset.h"
#include "gui/widgets/common/aabb.h"
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

protected:
    void updateGuiValues();

public:
    Gtk::Grid widget;

private:
    std::shared_ptr<SI::FrameSet> _frameSet;

    Gtk::Box _imageFilenameBox;
    Gtk::Entry _imageFilenameEntry;
    Gtk::Button _imageFilenameButton;

    UsizeSpinButtons _gridFrameSizeSpinButtons;
    UpointSpinButtons _gridOffsetSpinButtons;
    UsizeSpinButtons _gridPaddingSpinButtons;
    UpointSpinButtons _gridOriginSpinButtons;

    Gtk::Label _imageFilenameLabel;
    Gtk::Label _gridFrameSizeLabel, _gridFrameSizeCrossLabel;
    Gtk::Label _gridOffsetLabel, _gridOffsetCommaLabel;
    Gtk::Label _gridPaddingLabel, _gridPaddingCrossLabel;
    Gtk::Label _gridOriginLabel, _gridOriginCommaLabel;
};
}
}
}

#endif
