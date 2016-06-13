#pragma once

#include "namedlistdialog.h"
#include "gui/controllers/helpers/namedlistcontroller.h"
#include "gui/widgets/defaults.h"

#include <memory>

#include <glibmm/i18n.h>
#include <gtkmm.h>

namespace UnTech {
namespace Widgets {

template <class T, class ModelColumnsT>
class NamedListView {
    typedef typename Controller::NamedListController<T> NamedListController;

public:
    NamedListView(NamedListController& controller)
        : treeView()
        , _controller(controller)
        , columns()
    {
        treeModel = Gtk::ListStore::create(columns);

        sortedModel = Gtk::TreeModelSort::create(treeModel);
        columns.buildTreeViewColumns(treeView);

        treeView.set_model(sortedModel);
        treeView.set_sensitive(false);

        /*
         * SLOTS
         * =====
         */

        /* Controller */
        _controller.signal_selectedChanged().connect(sigc::mem_fun(
            *this, &NamedListView::onItemSelected));

        _controller.signal_dataChanged().connect(sigc::mem_fun(
            *this, &NamedListView::onItemChanged));

        _controller.signal_itemRenamed().connect(sigc::mem_fun(
            *this, &NamedListView::onItemRenamed));

        _controller.signal_listChanged().connect(sigc::mem_fun(
            *this, &NamedListView::rebuildTable));

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
    NamedListController& controller() { return _controller; }

    void onItemSelected()
    {
        const T* item = _controller.selected();

        if (item != nullptr) {
            const auto children = sortedModel->children();

            for (auto iter = children.begin(); iter != children.end(); ++iter) {
                auto row = *iter;

                if (row.get_value(columns.col_item) == item) {
                    _controller.setSelected(item);

                    treeView.get_selection()->select(row);

                    // scroll to selection
                    treeView.scroll_to_row(sortedModel->get_path(iter));
                    return;
                }
            }
        }

        // not found
        treeView.get_selection()->unselect_all();
    }

    void onItemRenamed(const T* item)
    {
        if (_controller.list()) {
            auto name = _controller.list()->getName(item);

            if (name.second) {
                for (auto row : treeModel->children()) {
                    if (row.get_value(columns.col_item) == item) {
                        row[columns.col_id] = name.first;
                        columns.setRowData(row, item);
                        return;
                    }
                }
            }
        }
    }

    void onItemChanged(const T* item)
    {
        if (_controller.list()) {
            if (_controller.list()->contains(item)) {
                for (auto row : treeModel->children()) {
                    if (row.get_value(columns.col_item) == item) {
                        columns.setRowData(row, item);
                        return;
                    }
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
            // BUGFIX: Calling treeModel->clear() segfaults
            // have to disconnect sortedModel first.

            treeView.unset_model();
            treeModel->clear();
            treeView.set_model(sortedModel);

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
        // while finding the selected item
        Gtk::TreeModel::iterator selectedRowIt;

        if (nItems > 0) {
            auto rowIt = treeModel->children().begin();
            for (const auto it : *list) {
                auto row = *rowIt;
                T* item = &it.second;

                columns.setRowData(row, item);
                row[columns.col_id] = it.first;
                row[columns.col_item] = item;

                if (item == selected) {
                    selectedRowIt = rowIt;
                }

                ++rowIt;
            }
        }

        // Highlight the selected item in the treeView
        if (selected != nullptr) {
            if (selectedRowIt) {
                auto row = sortedModel->convert_child_iter_to_iter(selectedRowIt);

                treeView.get_selection()->select(row);

                // scroll to selection
                treeView.scroll_to_row(sortedModel->get_path(row));
            }
            else {
                // If here then the item has been deleted
                treeView.get_selection()->unselect_all();
            }
        }
    }

public:
    Gtk::TreeView treeView;

protected:
    NamedListController& _controller;

    ModelColumnsT columns;
    Glib::RefPtr<Gtk::ListStore> treeModel;
    Glib::RefPtr<Gtk::TreeModelSort> sortedModel;
};

template <class T, class ModelColumnsT>
class NamedListEditor : public NamedListView<T, ModelColumnsT> {
    typedef typename Controller::NamedListController<T> NamedListController;

public:
    NamedListEditor(NamedListController& controller)
        : NamedListView<T, ModelColumnsT>(controller)
        , widget(Gtk::ORIENTATION_VERTICAL)
        , _treeContainer()
        , _buttonBox(Gtk::ORIENTATION_HORIZONTAL)
        , _createButton()
        , _cloneButton()
        , _renameButton()
        , _removeButton()
    {
        _createButton.set_image_from_icon_name("list-add");
        _cloneButton.set_image_from_icon_name("edit-copy");
        _renameButton.set_image_from_icon_name("insert-text");
        _removeButton.set_image_from_icon_name("list-remove");

        _createButton.set_tooltip_text(Glib::ustring::compose(
            _("Add %1"), T::TYPE_NAME));
        _cloneButton.set_tooltip_text(Glib::ustring::compose(
            _("Clone %1"), T::TYPE_NAME));
        _renameButton.set_tooltip_text(Glib::ustring::compose(
            _("Rename %1"), T::TYPE_NAME));
        _removeButton.set_tooltip_text(Glib::ustring::compose(
            _("Remove %1"), T::TYPE_NAME));

        _buttonBox.set_border_width(DEFAULT_BORDER);
        _buttonBox.set_layout(Gtk::BUTTONBOX_END);
        _buttonBox.add(_createButton);
        _buttonBox.add(_cloneButton);
        _buttonBox.add(_renameButton);
        _buttonBox.add(_removeButton);

        _buttonBox.set_child_non_homogeneous(_createButton, true);
        _buttonBox.set_child_non_homogeneous(_cloneButton, true);
        _buttonBox.set_child_non_homogeneous(_renameButton, true);
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
            *this, &NamedListEditor::updateButtonState));

        controller.signal_listChanged().connect(sigc::mem_fun(
            *this, &NamedListEditor::updateButtonState));

        controller.signal_listDataChanged().connect(sigc::hide(sigc::mem_fun(
            *this, &NamedListEditor::updateButtonState)));

        /* Button signals */
        _createButton.signal_clicked().connect([this](void) {
            const typename T::list_t* list = this->controller().list();

            if (list != nullptr) {
                NamedListDialog<T> dialog(
                    Glib::ustring::compose(_("Input name of new %1:"), T::TYPE_NAME),
                    widget);
                dialog.setList(list);

                auto ret = dialog.run();
                if (ret == Gtk::RESPONSE_ACCEPT) {
                    this->controller().create(dialog.get_text());
                }
            }
        });

        _cloneButton.signal_clicked().connect([this](void) {
            const typename T::list_t* list = this->controller().list();

            if (list != nullptr) {
                NamedListDialog<T> dialog(
                    Glib::ustring::compose(_("Input new name of cloned %1:"), T::TYPE_NAME),
                    widget);
                dialog.setList(list);
                dialog.set_text(this->controller().selectedName());

                auto ret = dialog.run();
                if (ret == Gtk::RESPONSE_ACCEPT) {
                    this->controller().selected_clone(dialog.get_text());
                }
            }
        });

        _renameButton.signal_clicked().connect([this](void) {
            const typename T::list_t* list = this->controller().list();
            const T* selected = this->controller().selected();

            if (list != nullptr && selected != nullptr) {
                NamedListDialog<T> dialog(
                    Glib::ustring::compose(_("Rename %1 to:"), T::TYPE_NAME),
                    widget);
                dialog.setItem(list, this->controller().selected());

                auto ret = dialog.run();
                if (ret == Gtk::RESPONSE_ACCEPT) {
                    this->controller().selected_rename(dialog.get_text());
                }
            }
        });

        _removeButton.signal_clicked().connect(sigc::mem_fun(
            this->controller(), &NamedListController::selected_remove));
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
            _renameButton.set_sensitive(enabled);
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
    Gtk::Button _renameButton;
    Gtk::Button _removeButton;
};
}
}
