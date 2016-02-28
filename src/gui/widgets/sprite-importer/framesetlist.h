#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_FRAMESETLIST_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_FRAMESETLIST_H_

#include "signals.h"
#include "gui/widgets/common/namedlist.h"
#include "models/sprite-importer/frameset.h"

#include <gtkmm.h>
#include <glibmm/i18n.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

namespace Private {

class FrameSetModelColumns : public Gtk::TreeModel::ColumnRecord {
public:
    FrameSetModelColumns()
    {
        add(col_item);
        add(col_id);
        add(col_image);
    }

    Gtk::TreeModelColumn<std::shared_ptr<SI::FrameSet>> col_item;
    Gtk::TreeModelColumn<std::string> col_id;
    Gtk::TreeModelColumn<std::string> col_image;

    inline void buildTreeViewColumns(Gtk::TreeView& treeView)
    {
        int nameId = treeView.append_column(_("Name"), col_id) - 1;
        Gtk::TreeViewColumn* nameColumn = treeView.get_column(nameId);
        nameColumn->set_sort_column(col_id);
        nameColumn->set_expand(true);
        nameColumn->set_resizable(true);

        int imageId = treeView.append_column(_("Image"), col_image) - 1;
        Gtk::TreeViewColumn* imageColumn = treeView.get_column(imageId);
        imageColumn->set_sort_column(col_image);
        imageColumn->set_resizable(true);

        treeView.set_headers_clickable(true);
        treeView.set_search_column(col_id);
    }

    inline void setRowData(Gtk::TreeRow& row, std::shared_ptr<SI::FrameSet> frameset)
    {
        row[col_image] = frameset->imageFilename();
    }

    inline static auto& signal_itemChanged() { return signal_frameSetChanged; }

    inline static auto& signal_listChanged() { return signal_frameSetListChanged; }

    inline static const char* itemTypeName() { return "Frameset"; }
};
}

typedef NamedListEditor<SI::FrameSet, Private::FrameSetModelColumns> FrameSetListEditor;
}
}
}

#endif
