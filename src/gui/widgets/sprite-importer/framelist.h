#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_FRAMELIST_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_FRAMELIST_H_

#include "signals.h"
#include "gui/widgets/common/namedlist.h"
#include "models/sprite-importer/frame.h"

#include <gtkmm.h>
#include <glibmm/i18n.h>

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

    Gtk::TreeModelColumn<std::shared_ptr<SI::Frame>> col_item;
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

    inline void setRowData(Gtk::TreeRow& row, std::shared_ptr<SI::Frame> frame)
    {
        row[col_x] = frame->location().x;
        row[col_y] = frame->location().y;
    }

    inline static auto& signal_itemChanged() { return Signals::frameChanged; }

    inline static auto& signal_listChanged() { return Signals::frameListChanged; }

    inline static const char* itemTypeName() { return "Frame"; }
};
}

typedef NamedListEditor<SI::Frame, Private::FrameModelColumns> FrameListEditor;
}
}
}

#endif
