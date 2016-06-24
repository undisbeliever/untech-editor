#pragma once

#include "gui/controllers/sprite-importer.h"
#include "gui/widgets/common/aabb.h"
#include "gui/widgets/common/colortogglebutton.h"
#include "gui/widgets/common/enumclasscombobox.h"
#include "gui/widgets/common/namedlistnameentry.h"
#include "gui/widgets/metasprite-common/abstractframesetpropertieseditor.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

class FrameSetPropertiesEditor {
public:
    FrameSetPropertiesEditor(SI::SpriteImporterController& controller);

protected:
    void updateGuiValues();

    void on_imageFilenameButtonClicked();

public:
    Gtk::Box widget;

private:
    SI::SpriteImporterController& _controller;

    MetaSpriteCommon::AbstractFrameSetPropertiesEditor _abstractEditor;

    Gtk::Grid _wgrid;

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
