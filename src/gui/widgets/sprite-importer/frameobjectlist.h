#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_FRAMEOBJECTLIST_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_FRAMEOBJECTLIST_H_

#include "gui/widgets/common/orderedlist.h"
#include "models/sprite-importer/frameobject.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

namespace Private {

class FrameObjectModelColumns : public Gtk::TreeModel::ColumnRecord {
public:
    FrameObjectModelColumns()
    {
        add(col_item);
        add(col_id);
        add(col_location);
        add(col_size);
    }

    Gtk::TreeModelColumn<std::shared_ptr<SI::FrameObject>> col_item;
    Gtk::TreeModelColumn<unsigned int> col_id;
    // ::TODO change to native types::
    Gtk::TreeModelColumn<Glib::ustring> col_location;
    Gtk::TreeModelColumn<Glib::ustring> col_size;

    inline void buildTreeViewColumns(Gtk::TreeView& treeView)
    {
        treeView.append_column("id", col_id);
        treeView.append_column("Location", col_location);
        treeView.append_column("Size", col_size);
    }

    inline void setRowData(Gtk::TreeRow& row, unsigned id, std::shared_ptr<SI::FrameObject> obj)
    {
        typedef UnTech::SpriteImporter::FrameObject::ObjectSize OS;

        row[col_id] = id;
        row[col_location] = Glib::ustring::compose("%1, %2", obj->location().x, obj->location().y);
        row[col_size] = obj->size() == OS::SMALL ? "Small" : "Large";
    }
};
}

typedef OrderedListEditor<SI::FrameObject, Private::FrameObjectModelColumns> FrameObjectListEditor;
}
}
}

#endif
