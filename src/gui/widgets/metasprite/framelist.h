#pragma once

#include "gui/widgets/common/namedlist.h"
#include "models/metasprite/frame.h"

#include <glibmm/i18n.h>
#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

namespace Private {

class FrameModelColumns : public Gtk::TreeModel::ColumnRecord {
public:
    FrameModelColumns()
    {
        add(col_item);
        add(col_id);
        add(col_x);
        add(col_y);
    }

    Gtk::TreeModelColumn<const MS::Frame*> col_item;
    Gtk::TreeModelColumn<std::string> col_id;
    Gtk::TreeModelColumn<unsigned int> col_x;
    Gtk::TreeModelColumn<unsigned int> col_y;

    inline void buildTreeViewColumns(Gtk::TreeView& treeView)
    {
        int nameId = treeView.append_column(_("Name"), col_id) - 1;
        Gtk::TreeViewColumn* nameColumn = treeView.get_column(nameId);
        nameColumn->set_sort_column(col_id);
        nameColumn->set_expand(true);

        treeView.set_headers_clickable(true);
        treeView.set_search_column(col_id);
    }

    inline void setRowData(Gtk::TreeRow&, const MS::Frame*)
    {
    }
};
}

typedef NamedListEditor<MS::Frame, Private::FrameModelColumns> FrameListEditor;
}
}
}
