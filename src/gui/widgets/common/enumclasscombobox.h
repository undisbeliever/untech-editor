#ifndef _UNTECH_GUI_WIDGETS_COMMON_ENUMCLASSCOMBOBOX_H
#define _UNTECH_GUI_WIDGETS_COMMON_ENUMCLASSCOMBOBOX_H

#include <map>
#include <string>

#include <gtkmm.h>
#include <glibmm/i18n.h>

namespace UnTech {
namespace Widgets {

template <class T>
class EnumClassComboBox : public Gtk::ComboBox {
public:
    EnumClassComboBox(const std::map<typename T::Enum, std::string>& mapping = T::enumMap)
        : Gtk::ComboBox()
        , _columns()
        , _treeModel(Gtk::ListStore::create(_columns))
    {
        for (const auto& m : mapping) {
            auto& row = *_treeModel->append();

            row[_columns.col_value] = m.first;
            row[_columns.col_name] = m.second;
        }

        set_model(_treeModel);
        pack_start(_columns.col_name);
    }

    T get_value() const
    {
        auto iter = get_active();

        if (iter) {
            return T(iter->get_value(_columns.col_value));
        }
        else {
            return T();
        }
    }

    void set_value(const T& e)
    {
        const auto children = _treeModel->children();

        for (auto iter = children.begin(); iter != children.end(); ++iter) {
            auto row = *iter;

            if (row[_columns.col_value] == e.value()) {
                set_active(row);
                return;
            }
        }

        unset_active();
    }

    inline void unset_value()
    {
        unset_active();
    }

private:
    class ModelColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        ModelColumns()
        {
            add(col_value);
            add(col_name);
        }

        Gtk::TreeModelColumn<typename T::Enum> col_value;
        Gtk::TreeModelColumn<Glib::ustring> col_name;
    };

    const ModelColumns _columns;
    const Glib::RefPtr<Gtk::ListStore> _treeModel;
};
}
}
#endif
