#pragma once

#include "gui/controllers/sprite-importer.h"
#include "gui/widgets/common/orderedlist.h"

#include <glibmm/i18n.h>
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

    Gtk::TreeModelColumn<const SI::FrameObject*> col_item;
    Gtk::TreeModelColumn<unsigned int> col_id;
    // ::TODO change to native types::
    Gtk::TreeModelColumn<Glib::ustring> col_location;
    Gtk::TreeModelColumn<Glib::ustring> col_size;

    inline void buildTreeViewColumns(Gtk::TreeView& treeView)
    {
        treeView.append_column(_("id"), col_id);
        treeView.append_column(_("Location"), col_location);
        treeView.append_column(_("Size"), col_size);
    }

    inline void setRowData(Gtk::TreeRow& row, const SI::FrameObject* obj)
    {
        typedef UnTech::SpriteImporter::FrameObject::ObjectSize OS;

        row[col_location] = Glib::ustring::compose("%1, %2", obj->location().x, obj->location().y);
        row[col_size] = obj->size() == OS::SMALL ? _("small") : _("large");
    }
};
}

typedef OrderedListEditor<SI::FrameObject, Private::FrameObjectModelColumns> FrameObjectListEditor;
}
}
}
