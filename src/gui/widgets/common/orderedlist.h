#ifndef _UNTECH_GUI_WIDGETS_COMMON_ORDEREDLIST_H
#define _UNTECH_GUI_WIDGETS_COMMON_ORDEREDLIST_H

#include "deleteconfirmationdialog.h"
#include "models/common/orderedlist.h"
#include "gui/widgets/defaults.h"

#include <memory>

#include <gtkmm.h>
#include <glibmm/i18n.h>

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
            std::shared_ptr<T> item = nullptr;

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

    inline void setList(typename T::list_t& newList)
    {
        setList(&newList);
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

    /**
     * Gets the first selected item, returns nullptr if none selected.
     */
    std::shared_ptr<T> getSelected()
    {
        return selected;
    }

    /**
     * Selects the given item.
     * Unselects everything if item is not in list.
     */
    void selectItem(std::shared_ptr<T> item)
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
    auto onItemChanged(std::shared_ptr<T> item)
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
            if (nItems > 0) {
                int id = 0;
                auto rowIt = treeModel->children().begin();
                for (auto item : *list) {
                    auto row = *rowIt;
                    columns.setRowData(row, item);
                    row[columns.col_id] = id;
                    row[columns.col_item] = item;

                    ++id;
                    ++rowIt;
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

    std::shared_ptr<T> selected;
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
            auto item = this->list->create();

            this->emitListChangedSignal();
            this->selectItem(item);
        });

        _cloneButton.signal_clicked().connect([this](void) {
            auto item = this->list->clone(this->getSelected());

            this->emitListChangedSignal();
            this->selectItem(item);
        });

        _moveUpButton.signal_clicked().connect([this](void) {
            auto item = this->getSelected();

            this->list->moveUp(this->getSelected());

            this->emitListChangedSignal();
            this->selectItem(item);
        });

        _moveDownButton.signal_clicked().connect([this](void) {
            auto item = this->getSelected();

            this->list->moveDown(this->getSelected());

            this->emitListChangedSignal();
            this->selectItem(item);
        });

        _removeButton.signal_clicked().connect([this](void) {
            auto toDelete = this->getSelected();

            DeleteConfirmationDialog dialog(
                this->columns.itemTypeName(), widget);

            auto ret = dialog.run();
            if (ret == Gtk::RESPONSE_ACCEPT) {
                this->list->remove(toDelete);
                this->emitListChangedSignal();
                this->selectItem(nullptr);
            }
        });

        this->signal_selected_changed().connect(sigc::mem_fun(*this, &OrderedListEditor::updateButtonState));
    }

    inline void setList(typename T::list_t& newList)
    {
        setList(&newList);
    }

    inline void setList(typename T::list_t* newList)
    {
        OrderedListView<T, ModelColumnsT>::setList(newList);

        updateButtonState();
    }

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
