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

class EntityHitboxModelColumns : public Gtk::TreeModel::ColumnRecord {
public:
    EntityHitboxModelColumns()
    {
        add(col_item);
        add(col_id);
        add(col_aabb);
        add(col_parameter);
    }

    Gtk::TreeModelColumn<const SI::EntityHitbox*> col_item;
    Gtk::TreeModelColumn<unsigned int> col_id;
    // ::TODO change to native types::
    Gtk::TreeModelColumn<Glib::ustring> col_aabb;
    Gtk::TreeModelColumn<unsigned int> col_parameter;

    inline void buildTreeViewColumns(Gtk::TreeView& treeView)
    {
        treeView.append_column(_("id"), col_id);
        treeView.append_column(_("AABB"), col_aabb);
        treeView.append_column(_("Parameter"), col_parameter);
    }

    inline void setRowData(Gtk::TreeRow& row, const SI::EntityHitbox* eh)
    {
        auto aabb = eh->aabb();
        row[col_aabb] = Glib::ustring::compose("%1, %2 : %3 x %4",
                                               aabb.x, aabb.y, aabb.width, aabb.height);
        row[col_parameter] = eh->parameter();
    }
};
}

typedef OrderedListEditor<SI::EntityHitbox, Private::EntityHitboxModelColumns> EntityHitboxListEditor;
}
}
}
