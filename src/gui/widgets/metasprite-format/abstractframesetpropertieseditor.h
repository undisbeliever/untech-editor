#pragma once

#include "framesetexportorderrotreeview.h"
#include "gui/controllers/metasprite-format.h"
#include "gui/widgets/common/enumclasscombobox.h"
#include "gui/widgets/common/namedlistnameentry.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace MetaSpriteFormat {

namespace MSF = UnTech::MetaSpriteFormat;

class AbstractFrameSetPropertiesEditor {
public:
    AbstractFrameSetPropertiesEditor(MSF::AbstractFrameSetController& controller);

protected:
    void updateGuiValues();
    void updateGuiTree();

    void on_fseoFilenameButtonClicked();

    void loadFrameSetExportOrderFile(const Glib::ustring& filename);

public:
    Gtk::Box widget;

private:
    MSF::AbstractFrameSetController& _controller;

    Gtk::Grid _grid;

    NamedListNameEntry _nameEntry;
    EnumClassComboBox<MSF::TilesetType> _tilesetTypeCombo;

    Gtk::Box _fseoFilenameBox;
    Gtk::Entry _fseoFilenameEntry;
    Gtk::Button _fseoFilenameButton;
    Gtk::Entry _fseoNameEntry;

    Gtk::ScrolledWindow _fseoContainer;
    FrameSetExportOrderRoTreeView _fseoTreeView;

    Gtk::Label _nameLabel;
    Gtk::Label _tilesetTypeLabel;
    Gtk::Label _fseoFilenameLabel;
    Gtk::Label _fseoNameLabel;

    // ::SHOULDDO animation selection::

    bool _updatingValues;
};
}
}
}
