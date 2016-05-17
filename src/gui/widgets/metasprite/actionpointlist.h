#ifndef _UNTECH_GUI_WIDGETS_METASPRITE_ACTIONPOINTLIST_H_
#define _UNTECH_GUI_WIDGETS_METASPRITE_ACTIONPOINTLIST_H_

#include "signals.h"
#include "gui/widgets/common/orderedlist.h"
#include "models/metasprite/actionpoint.h"

#include <glibmm/i18n.h>
#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

namespace Private {

class ActionPointModelColumns : public Gtk::TreeModel::ColumnRecord {
public:
    ActionPointModelColumns()
    {
        add(col_item);
        add(col_id);
        add(col_location);
        add(col_parameter);
    }

    Gtk::TreeModelColumn<MS::ActionPoint*> col_item;
    Gtk::TreeModelColumn<unsigned int> col_id;
    // ::TODO change to native types::
    Gtk::TreeModelColumn<Glib::ustring> col_location;
    Gtk::TreeModelColumn<unsigned int> col_parameter;

    inline void buildTreeViewColumns(Gtk::TreeView& treeView)
    {
        treeView.append_column(_("id"), col_id);
        treeView.append_column(_("Location"), col_location);
        treeView.append_column(_("Parameter"), col_parameter);
    }

    inline void setRowData(Gtk::TreeRow& row, const MS::ActionPoint* ap)
    {
        row[col_location] = Glib::ustring::compose("%1, %2", ap->location().x, ap->location().y);
        row[col_parameter] = ap->parameter();
    }

    inline static auto& signal_itemChanged() { return Signals::actionPointChanged; }

    inline static auto& signal_listChanged() { return Signals::actionPointListChanged; }

    inline static const char* itemTypeName() { return "Action Point"; }
};
}

typedef OrderedListEditor<MS::ActionPoint, Private::ActionPointModelColumns> ActionPointListEditor;
}
}
}

#endif
