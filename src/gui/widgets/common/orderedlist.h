#ifndef _UNTECH_GUI_WIDGETS_COMMON_ORDEREDLIST_H
#define _UNTECH_GUI_WIDGETS_COMMON_ORDEREDLIST_H

#include "models/common/orderedlist.h"
#include "gui/widgets/defaults.h"
#include "gui/widgets/sprite-importer/signals.h"

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
            if (list != nullptr) {
                rebuildTable();

                treeView.set_sensitive(true);
            }
            else {
                treeModel->clear();
                treeView.set_sensitive(false);
            }
        }
    }

    /**
     * Gets the first selected item, returns nullptr if none selected.
     */
    std::shared_ptr<T> getSelected()
    {
        auto rowIt = treeView.get_selection()->get_selected();
        if (rowIt) {
            auto row = *rowIt;
            return row[columns.col_item];
        }
        else {
            return nullptr;
        }
    }

    /**
     * Selects the given item.
     * Unselects everything if item is not in list.
     */
    void selectItem(std::shared_ptr<T> item)
    {
        const auto children = treeModel->children();

        for (auto iter = children.begin(); iter != children.end(); ++iter) {
            auto row = *iter;

            if (row.get_value(columns.col_item) == item) {
                treeView.get_selection()->select(row);

                // scroll to selection
                treeView.scroll_to_row(treeModel->get_path(iter));
                return;
            }
        }

        // not found
        treeView.get_selection()->unselect_all();
    }

    inline auto signal_selected_changed() { return treeView.get_selection()->signal_changed(); }

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
                --rowIt;
                for (size_t i = 0; i < (nRows - nItems); i++) {
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
};

template <class T, class ModelColumnsT>
class OrderedListEditor : public OrderedListView<T, ModelColumnsT> {

public:
    OrderedListEditor()
        : OrderedListView<T, ModelColumnsT>()
        , widget(Gtk::ORIENTATION_VERTICAL)
        , _treeContainer()
        , _buttonBox(Gtk::ORIENTATION_HORIZONTAL)
        , _createButton(_("Create"))
        , _cloneButton(_("Clone"))
        , _moveUpButton(_("Up"))
        , _moveDownButton(_("Down"))
        , _removeButton(_("Remove"))
    {
        // ::TODO button icons and tool tips::

        updateButtonState();
        _buttonBox.set_border_width(DEFAULT_BORDER);
        _buttonBox.set_layout(Gtk::BUTTONBOX_END);
        _buttonBox.add(_createButton);
        _buttonBox.add(_cloneButton);
        _buttonBox.add(_moveUpButton);
        _buttonBox.add(_moveDownButton);
        _buttonBox.add(_removeButton);
        widget.pack_start(_buttonBox, Gtk::PACK_SHRINK);

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
            this->list->remove(this->getSelected());
            this->emitListChangedSignal();
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
