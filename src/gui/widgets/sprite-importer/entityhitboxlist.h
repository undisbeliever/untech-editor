#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_ENTITYHITBOXLIST_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_ENTITYHITBOXLIST_H_

#include "signals.h"
#include "gui/widgets/common/orderedlist.h"
#include "models/sprite-importer/actionpoint.h"

#include <gtkmm.h>
#include <glibmm/i18n.h>

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

    Gtk::TreeModelColumn<SI::EntityHitbox*> col_item;
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

    inline static auto& signal_itemChanged() { return Signals::entityHitboxChanged; }

    inline static auto& signal_listChanged() { return Signals::entityHitboxListChanged; }

    inline static const char* itemTypeName() { return "Hitbox"; }
};
}

typedef OrderedListEditor<SI::EntityHitbox, Private::EntityHitboxModelColumns> EntityHitboxListEditor;
}
}
}

#endif
