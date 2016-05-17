#ifndef _UNTECH_GUI_WIDGETS_COMMON_NAMEDLIST_H
#define _UNTECH_GUI_WIDGETS_COMMON_NAMEDLIST_H

#include "namedlistdialog.h"
#include "gui/undo/namedlistactions.h"
#include "gui/widgets/defaults.h"
#include "models/common/namedlist.h"

#include <memory>

#include <glibmm/i18n.h>
#include <gtkmm.h>

namespace UnTech {
namespace Widgets {

template <class T, class ModelColumnsT>
class NamedListView {

public:
    NamedListView()
        : treeView()
        , columns()
        , list(nullptr)
        , selected(nullptr)
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
            *this, &NamedListView::onItemChanged));

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
                // BUGFIX: Calling treeModel->clear() segfaults
                // have to disconnect sortedModel first.

                treeView.unset_model();
                treeModel->clear();
                treeView.set_model(sortedModel);

                treeView.set_sensitive(list != nullptr);
            }
        }
    }

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
                const auto children = sortedModel->children();

                for (auto iter = children.begin(); iter != children.end(); ++iter) {
                    auto row = *iter;

                    if (row.get_value(columns.col_item) == item) {
                        selected = item;

                        treeView.get_selection()->select(row);

                        // scroll to selection
                        treeView.scroll_to_row(sortedModel->get_path(iter));
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
        Gtk::TreeModel::iterator selectedRowIt;

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
            if (selected) {
                if (selectedRowIt) {
                    auto row = sortedModel->convert_child_iter_to_iter(selectedRowIt);

                    treeView.get_selection()->select(row);

                    // scroll to selection
                    treeView.scroll_to_row(sortedModel->get_path(row));
                    _signal_selected_changed.emit();
                }
                else {
                    // If here then the item has been deleted
                    selected = nullptr;
                    treeView.get_selection()->unselect_all();
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
    Glib::RefPtr<Gtk::TreeModelSort> sortedModel;

    typename T::list_t* list;

    T* selected;
    sigc::signal<void> _signal_selected_changed;
};

template <class T, class ModelColumnsT>
class NamedListEditor : public NamedListView<T, ModelColumnsT> {

public:
    NamedListEditor()
        : NamedListView<T, ModelColumnsT>()
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

        _createButton.set_tooltip_text(Glib::ustring::compose(_("Add %1"), this->columns.itemTypeName()));
        _cloneButton.set_tooltip_text(Glib::ustring::compose(_("Clone %1"), this->columns.itemTypeName()));
        _renameButton.set_tooltip_text(Glib::ustring::compose(_("Rename %1"), this->columns.itemTypeName()));
        _removeButton.set_tooltip_text(Glib::ustring::compose(_("Remove %1"), this->columns.itemTypeName()));

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
         * SIGNALS
         */
        _createButton.signal_clicked().connect([this](void) {
            NamedListDialog<T> dialog(
                Glib::ustring::compose(_("Input name of new %1:"), this->columns.itemTypeName()),
                widget);
            dialog.setList(this->list);

            auto ret = dialog.run();
            if (ret == Gtk::RESPONSE_ACCEPT) {
                auto item = Undo::namedList_create<T>(this->list, dialog.get_text(),
                                                      this->columns.signal_listChanged(),
                                                      _createButton.get_tooltip_text());
                this->selectItem(item);
            }
        });

        _cloneButton.signal_clicked().connect([this](void) {
            T* toCopy = this->getSelected();

            NamedListDialog<T> dialog(
                Glib::ustring::compose(_("Input new name of cloned %1:"), this->columns.itemTypeName()),
                widget);
            dialog.setList(this->list);

            auto ret = dialog.run();
            if (ret == Gtk::RESPONSE_ACCEPT) {
                auto newName = dialog.get_text();
                auto item = Undo::namedList_clone<T>(this->list, toCopy, dialog.get_text(),
                                                     this->columns.signal_listChanged(),
                                                     _cloneButton.get_tooltip_text());
                this->selectItem(item);
            }
        });

        _renameButton.signal_clicked().connect([this](void) {
            T* toRename = this->getSelected();

            NamedListDialog<T> dialog(
                Glib::ustring::compose(_("Rename %1 to:"), this->columns.itemTypeName()),
                widget);
            dialog.setItem(this->list, toRename);

            auto ret = dialog.run();
            if (ret == Gtk::RESPONSE_ACCEPT) {
                Undo::namedList_rename<T>(this->list, toRename, dialog.get_text(),
                                          this->columns.signal_listChanged(),
                                          _renameButton.get_tooltip_text());
            }
        });

        _removeButton.signal_clicked().connect([this](void) {
            T* toRemove = this->getSelected();

            this->selectItem(nullptr);

            Undo::namedList_remove<T>(this->list, toRemove,
                                      this->columns.signal_listChanged(),
                                      _removeButton.get_tooltip_text());
        });

        this->columns.signal_listChanged().connect([this](const typename T::list_t* list) {
            if (list == this->list) {
                updateButtonState();
            }
        });

        this->signal_selected_changed().connect(sigc::mem_fun(*this, &NamedListEditor::updateButtonState));
    }

    inline void setList(typename T::list_t& newList)
    {
        setList(&newList);
    }

    inline void setList(typename T::list_t* newList)
    {
        NamedListView<T, ModelColumnsT>::setList(newList);

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
#endif
