#pragma once

#include "gui/controllers/helpers/orderedlistcontroller.h"
#include "gui/widgets/defaults.h"

#include <memory>

#include <glibmm/i18n.h>
#include <gtkmm.h>

namespace UnTech {
namespace Widgets {

template <class T, class ModelColumnsT>
class OrderedListView {
    typedef typename Controller::OrderedListController<T> OrderedListController;

public:
    OrderedListView(OrderedListController& controller)
        : treeView()
        , _controller(controller)
        , columns()
    {
        treeModel = Gtk::ListStore::create(columns);
        columns.buildTreeViewColumns(treeView);

        treeView.set_model(treeModel);
        treeView.set_sensitive(false);

        /*
         * SLOTS
         * =====
         */

        /* Controller */
        _controller.signal_selectedChanged().connect(sigc::mem_fun(
            *this, &OrderedListView::onItemSelected));

        _controller.signal_dataChanged().connect(sigc::mem_fun(
            *this, &OrderedListView::onItemChanged));

        _controller.signal_listChanged().connect(sigc::mem_fun(
            *this, &OrderedListView::rebuildTable));

        _controller.signal_listDataChanged().connect(
            [this](const typename T::list_t* list) {
                if (list == _controller.list()) {
                    this->rebuildTable();
                }
            });

        /* Handle change in selection */
        treeView.get_selection()->signal_changed().connect([this](void) {
            const T* item = nullptr;

            auto rowIt = treeView.get_selection()->get_selected();
            if (rowIt) {
                auto row = *rowIt;
                item = row[columns.col_item];
            }
            _controller.setSelected(item);
        });
    }

protected:
    OrderedListController& controller() { return _controller; }

    void onItemSelected()
    {
        const T* item = _controller.selected();

        if (item != nullptr) {
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
        }

        // not found
        treeView.get_selection()->unselect_all();
    }

    auto onItemChanged(const T* item)
    {
        if (_controller.list()) {
            for (auto row : treeModel->children()) {
                if (row.get_value(columns.col_item) == item) {
                    columns.setRowData(row, item);
                    return;
                }
            }
        }
    }

    void rebuildTable()
    {
        const typename T::list_t* list = _controller.list();
        const T* selected = _controller.selected();

        treeView.set_sensitive(list != nullptr);

        if (list == nullptr || list->size() == 0) {
            treeModel->clear();
            treeView.set_sensitive(list != nullptr);
            return;
        }

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
        bool found = false;

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
        }
    }

public:
    Gtk::TreeView treeView;

protected:
    OrderedListController& _controller;

    ModelColumnsT columns;
    Glib::RefPtr<Gtk::ListStore> treeModel;
};

template <class T, class ModelColumnsT>
class OrderedListEditor : public OrderedListView<T, ModelColumnsT> {
    typedef typename Controller::OrderedListController<T> OrderedListController;

public:
    OrderedListEditor(OrderedListController& controller)
        : OrderedListView<T, ModelColumnsT>(controller)
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

        _createButton.set_tooltip_text(Glib::ustring::compose(
            _("Add %1"), T::TYPE_NAME));
        _cloneButton.set_tooltip_text(Glib::ustring::compose(
            _("Clone %1"), T::TYPE_NAME));
        _moveUpButton.set_tooltip_text(Glib::ustring::compose(
            _("Move %1 up"), T::TYPE_NAME));
        _moveDownButton.set_tooltip_text(Glib::ustring::compose(
            _("Move %1 down"), T::TYPE_NAME));
        _removeButton.set_tooltip_text(Glib::ustring::compose(
            _("Remove %1"), T::TYPE_NAME));

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
         * SLOTS
         */
        /* Controller signals */
        controller.signal_selectedChanged().connect(sigc::mem_fun(
            *this, &OrderedListEditor::updateButtonState));

        controller.signal_listChanged().connect(sigc::mem_fun(
            *this, &OrderedListEditor::updateButtonState));

        controller.signal_listDataChanged().connect(sigc::hide(sigc::mem_fun(
            *this, &OrderedListEditor::updateButtonState)));

        /* Button signals */
        _createButton.signal_clicked().connect(sigc::mem_fun(
            this->controller(), &OrderedListController::create));

        _cloneButton.signal_clicked().connect(sigc::mem_fun(
            this->controller(), &OrderedListController::selected_clone));

        _moveUpButton.signal_clicked().connect(sigc::mem_fun(
            this->controller(), &OrderedListController::selected_moveUp));

        _moveDownButton.signal_clicked().connect(sigc::mem_fun(
            this->controller(), &OrderedListController::selected_moveDown));

        _removeButton.signal_clicked().connect(sigc::mem_fun(
            this->controller(), &OrderedListController::selected_remove));
    }

protected:
    void updateButtonState()
    {
        const typename T::list_t* list = this->controller().list();
        const T* item = this->controller().selected();

        if (list != nullptr) {
            _buttonBox.set_sensitive(true);

            bool enabled = item != nullptr;

            _createButton.set_sensitive(true);
            _cloneButton.set_sensitive(enabled);
            _moveUpButton.set_sensitive(enabled && !(list->isFirst(item)));
            _moveDownButton.set_sensitive(enabled && !(list->isLast(item)));
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
