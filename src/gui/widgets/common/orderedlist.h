#ifndef _UNTECH_GUI_WIDGETS_COMMON_ORDEREDLIST_H
#define _UNTECH_GUI_WIDGETS_COMMON_ORDEREDLIST_H

#include "gui/undo/orderedlistactions.h"
#include "gui/widgets/defaults.h"
#include "models/common/orderedlist.h"

#include <memory>

#include <glibmm/i18n.h>
#include <gtkmm.h>

namespace UnTech {
namespace Widgets {

template <class T, class ModelColumnsT>
class OrderedListView {

public:
    OrderedListView()
        : treeView()
        , columns()
        , list(nullptr)
        , selected(nullptr)
    {
        treeModel = Gtk::ListStore::create(columns);
        columns.buildTreeViewColumns(treeView);

        treeView.set_model(treeModel);
        treeView.set_sensitive(false);

        /*
         * SLOTS
         * =====
         */
        /* Handle change in selection */
        treeView.get_selection()->signal_changed().connect([this](void) {
            T* item = nullptr;

            auto rowIt = treeView.get_selection()->get_selected();
            if (rowIt) {
                auto row = *rowIt;
                item = row[columns.col_item];
            }
            if (selected != item) {
                selected = item;
                _signal_selected_changed.emit();
            }
        });

        /* Update GUI if item has changed */
        columns.signal_itemChanged().connect(sigc::mem_fun(
            *this, &OrderedListView::onItemChanged));

        /* Rebuild table if list has changed */
        columns.signal_listChanged().connect([this](const typename T::list_t* list) {
            if (this->list == list) {
                rebuildTable();
            }
        });
    }

    void setList(typename T::list_t* newList)
    {
        if (list != newList) {
            list = newList;

            // Prevent race condition.
            // Select NULL item before updating model
            selectItem(nullptr);

            if (list != nullptr && list->size() > 0) {
                rebuildTable();

                treeView.set_sensitive(true);
            }
            else {
                treeModel->clear();

                treeView.set_sensitive(list != nullptr);
            }
        }
    }
    inline void setList(typename T::list_t& newList) { setList(&newList); }

    /**
     * Gets the first selected item, returns nullptr if none selected.
     */
    T* getSelected() { return selected; }

    /**
     * Selects the given item.
     * Unselects everything if item is not in list.
     */
    void selectItem(T* item)
    {
        if (item != selected) {
            if (item != nullptr) {
                const auto children = treeModel->children();

                for (auto iter = children.begin(); iter != children.end(); ++iter) {
                    auto row = *iter;

                    if (row.get_value(columns.col_item) == item) {
                        selected = item;

                        treeView.get_selection()->select(row);

                        // scroll to selection
                        treeView.scroll_to_row(treeModel->get_path(iter));
                        _signal_selected_changed.emit();
                        return;
                    }
                }
            }

            // not found
            if (selected != nullptr) {
                selected = nullptr;

                treeView.get_selection()->unselect_all();
                _signal_selected_changed.emit();
            }
        }
    }

    inline auto signal_selected_changed() { return _signal_selected_changed; }

protected:
    inline void emitListChangedSignal()
    {
        columns.signal_listChanged().emit(list);
    }

    /**
     * Called when an item has changed, updates the field.
     */
    auto onItemChanged(const T* item)
    {
        for (auto row : treeModel->children()) {
            if (row.get_value(columns.col_item) == item) {
                columns.setRowData(row, item);
            }
        }
    }

    void rebuildTable()
    {
        if (list != nullptr) {
            bool found = false;

            // resize model
            size_t nItems = list->size();
            size_t nRows = treeModel->children().size();

            if (nItems > nRows) {
                // grow model
                for (size_t i = 0; i < (nItems - nRows); i++) {
                    treeModel->append();
                }
            }
            else if (nItems < nRows) {
                // shrink model
                auto rowIt = treeModel->children().end();
                for (size_t i = 0; i < (nRows - nItems); i++) {
                    --rowIt;
                    rowIt = treeModel->erase(rowIt);
                }
            }

            // fill with data
            // change selection if necessary
            if (nItems > 0) {
                int id = 0;
                auto rowIt = treeModel->children().begin();
                for (T& itemRef : *list) {
                    auto row = *rowIt;
                    T* item = &itemRef;

                    columns.setRowData(row, item);
                    row[columns.col_id] = id;
                    row[columns.col_item] = item;

                    if (item == selected) {
                        treeView.get_selection()->select(rowIt);
                        found = true;
                    }

                    ++id;
                    ++rowIt;
                }
            }

            if (!found) {
                treeView.get_selection()->unselect_all();
                if (selected) {
                    selected = nullptr;
                    _signal_selected_changed.emit();
                }
            }
        }
    }

public:
    Gtk::TreeView treeView;

protected:
    ModelColumnsT columns;
    Glib::RefPtr<Gtk::ListStore> treeModel;

    typename T::list_t* list;

    T* selected;
    sigc::signal<void> _signal_selected_changed;
};

template <class T, class ModelColumnsT>
class OrderedListEditor : public OrderedListView<T, ModelColumnsT> {

public:
    OrderedListEditor()
        : OrderedListView<T, ModelColumnsT>()
        , widget(Gtk::ORIENTATION_VERTICAL)
        , _treeContainer()
        , _buttonBox(Gtk::ORIENTATION_HORIZONTAL)
        , _createButton()
        , _cloneButton()
        , _moveUpButton()
        , _moveDownButton()
        , _removeButton()
    {
        _createButton.set_image_from_icon_name("list-add");
        _cloneButton.set_image_from_icon_name("edit-copy");
        _moveUpButton.set_image_from_icon_name("go-up");
        _moveDownButton.set_image_from_icon_name("go-down");
        _removeButton.set_image_from_icon_name("list-remove");

        _createButton.set_tooltip_text(Glib::ustring::compose(_("Add %1"), this->columns.itemTypeName()));
        _cloneButton.set_tooltip_text(Glib::ustring::compose(_("Clone %1"), this->columns.itemTypeName()));
        _moveUpButton.set_tooltip_text(Glib::ustring::compose(_("Move %1 up"), this->columns.itemTypeName()));
        _moveDownButton.set_tooltip_text(Glib::ustring::compose(_("Move %1 down"), this->columns.itemTypeName()));
        _removeButton.set_tooltip_text(Glib::ustring::compose(_("Remove %1"), this->columns.itemTypeName()));

        _buttonBox.set_border_width(DEFAULT_BORDER);
        _buttonBox.set_layout(Gtk::BUTTONBOX_END);
        _buttonBox.pack_start(_createButton, Gtk::PACK_SHRINK);
        _buttonBox.pack_start(_cloneButton, Gtk::PACK_SHRINK);
        _buttonBox.pack_start(_moveUpButton, Gtk::PACK_SHRINK);
        _buttonBox.pack_start(_moveDownButton, Gtk::PACK_SHRINK);
        _buttonBox.pack_start(_removeButton, Gtk::PACK_SHRINK);

        _buttonBox.set_child_non_homogeneous(_createButton, true);
        _buttonBox.set_child_non_homogeneous(_cloneButton, true);
        _buttonBox.set_child_non_homogeneous(_moveUpButton, true);
        _buttonBox.set_child_non_homogeneous(_moveDownButton, true);
        _buttonBox.set_child_non_homogeneous(_removeButton, true);

        widget.pack_start(_buttonBox, Gtk::PACK_SHRINK);

        updateButtonState();

        _treeContainer.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
        _treeContainer.add(this->treeView);
        widget.pack_start(_treeContainer, Gtk::PACK_EXPAND_WIDGET);

        /**
         * SIGNALS
         */
        _createButton.signal_clicked().connect([this](void) {
            auto item = Undo::orderedList_create<T>(this->list,
                                                    this->columns.signal_listChanged(),
                                                    _createButton.get_tooltip_text());

            this->selectItem(item);
        });

        _cloneButton.signal_clicked().connect([this](void) {
            auto item = Undo::orderedList_clone(this->list, this->getSelected(),
                                                this->columns.signal_listChanged(),
                                                _cloneButton.get_tooltip_text());

            this->selectItem(item);
        });

        _moveUpButton.signal_clicked().connect([this](void) {
            Undo::orderedList_moveUp(this->list, this->getSelected(),
                                     this->columns.signal_listChanged(),
                                     _moveUpButton.get_tooltip_text());
        });

        _moveDownButton.signal_clicked().connect([this](void) {
            Undo::orderedList_moveDown(this->list, this->getSelected(),
                                       this->columns.signal_listChanged(),
                                       _moveDownButton.get_tooltip_text());
        });

        _removeButton.signal_clicked().connect([this](void) {
            T* toRemove = this->getSelected();

            this->selectItem(nullptr);

            Undo::orderedList_remove(this->list, toRemove,
                                     this->columns.signal_listChanged(),
                                     _removeButton.get_tooltip_text());
        });

        this->columns.signal_listChanged().connect([this](const typename T::list_t* list) {
            if (list == this->list) {
                updateButtonState();
            }
        });

        this->signal_selected_changed().connect(sigc::mem_fun(*this,
                                                              &OrderedListEditor::updateButtonState));
    }

    inline void setList(typename T::list_t* newList)
    {
        OrderedListView<T, ModelColumnsT>::setList(newList);

        updateButtonState();
    }
    inline void setList(typename T::list_t& newList) { setList(&newList); }

protected:
    void updateButtonState()
    {
        if (this->list) {
            _buttonBox.set_sensitive(true);

            auto item = this->getSelected();

            bool enabled = item != nullptr;

            _createButton.set_sensitive(true);
            _cloneButton.set_sensitive(enabled);
            _moveUpButton.set_sensitive(enabled && !(this->list->isFirst(item)));
            _moveDownButton.set_sensitive(enabled && !(this->list->isLast(item)));
            _removeButton.set_sensitive(enabled);
        }
        else {
            _buttonBox.set_sensitive(false);
        }
    }

public:
    Gtk::Box widget;

private:
    Gtk::ScrolledWindow _treeContainer;

    Gtk::ButtonBox _buttonBox;
    Gtk::Button _createButton;
    Gtk::Button _cloneButton;
    Gtk::Button _moveUpButton;
    Gtk::Button _moveDownButton;
    Gtk::Button _removeButton;
};
}
}
#endif
