#pragma once

#include "models/metasprite-format/framesetexportorder.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace MetaSpriteFormat {

namespace {
namespace FSEO = UnTech::MetaSpriteFormat::FrameSetExportOrder;
}

class FrameSetExportOrderRoTreeView : public Gtk::TreeView {

private:
    class ModelColumns : protected Gtk::TreeModel::ColumnRecord {
    public:
        ModelColumns();

        const Gtk::TreeModel::ColumnRecord& record() const { return *this; }

        Gtk::TreeModelColumn<unsigned> pos;
        Gtk::TreeModelColumn<Glib::ustring> name;
    };

public:
    FrameSetExportOrderRoTreeView();

    void clear() { _treeStore->clear(); }

    void loadData(std::shared_ptr<const FSEO::ExportOrderDocument> document);

private:
    const ModelColumns _columns;
    const Glib::RefPtr<Gtk::TreeStore> _treeStore;
};
}
}
}
