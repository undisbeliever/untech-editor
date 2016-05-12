#ifndef _UNTECH_GUI_WIDGETS_METASPRITEFORMAT_METASPRITEFORMATEDITOR_H
#define _UNTECH_GUI_WIDGETS_METASPRITEFORMAT_METASPRITEFORMATEDITOR_H

#include "framesetexportorderrotreeview.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace MetaSpriteFormat {

namespace {
namespace FSEO = UnTech::MetaSpriteFormat::FrameSetExportOrder;
}

class MetaSpriteFormatEditor : public Gtk::Paned {
public:
    MetaSpriteFormatEditor();

    const std::string& exportOrderFilename() const;

    void setFrameSetExportOrderDocument(const std::shared_ptr<const FSEO::ExportOrderDocument>& document);
    void loadFrameSetExportOrderFile(const Glib::ustring& filename);

protected:
    void updateGuiValues();

    void on_fseFilenameButtonClicked();

public:
    sigc::signal<void> signal_frameSetExportOrderLoaded;

private:
    std::shared_ptr<const FSEO::ExportOrderDocument> _fseoDocument;

    Gtk::Box _fseoBox;
    Gtk::Grid _fseoGrid;

    Gtk::Box _fseoFilenameBox;
    Gtk::Entry _fseoFilenameEntry;
    Gtk::Button _fseoFilenameButton;

    Gtk::Entry _fseoNameEntry;

    Gtk::ScrolledWindow _fseoContainer;
    FrameSetExportOrderRoTreeView _fseoTreeView;

    Gtk::Label _fseoFilenameLabel;
    Gtk::Label _fseoNameLabel;

    // ::TODO animation editor::
};
}
}
}
#endif
