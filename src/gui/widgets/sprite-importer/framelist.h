#pragma once

#include "gui/controllers/sprite-importer.h"
#include "gui/widgets/common/namedlist.h"

#include <glibmm/i18n.h>
#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

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

    Gtk::TreeModelColumn<const SI::Frame*> col_item;
    Gtk::TreeModelColumn<std::string> col_id;
    Gtk::TreeModelColumn<unsigned int> col_x;
    Gtk::TreeModelColumn<unsigned int> col_y;

    inline void buildTreeViewColumns(Gtk::TreeView& treeView)
    {
        int nameId = treeView.append_column(_("Name"), col_id) - 1;
        Gtk::TreeViewColumn* nameColumn = treeView.get_column(nameId);
        nameColumn->set_sort_column(col_id);
        nameColumn->set_expand(true);

        int xId = treeView.append_column(_("X"), col_x) - 1;
        Gtk::TreeViewColumn* xColumn = treeView.get_column(xId);
        xColumn->set_sort_column(col_x);

        int yId = treeView.append_column(_("Y"), col_y) - 1;
        Gtk::TreeViewColumn* yColumn = treeView.get_column(yId);
        yColumn->set_sort_column(col_y);

        treeView.set_headers_clickable(true);
        treeView.set_search_column(col_id);
    }

    inline void setRowData(Gtk::TreeRow& row, const SI::Frame* frame)
    {
        row[col_x] = frame->location().x;
        row[col_y] = frame->location().y;
    }
};
}

typedef NamedListEditor<SI::Frame, Private::FrameModelColumns> FrameListEditor;
}
}
}
