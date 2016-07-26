#pragma once
#include "mylistctrl.h"
#include "namedlistnamedialiog.h"
#include "gui/controllers/helpers/namedlistcontroller.h"
#include "gui/view/defaults.h"
#include "models/common/optional.h"
#include <cassert>
#include <vector>
#include <wx/artprov.h>
#include <wx/listctrl.h>
#include <wx/toolbar.h>
#include <wx/wx.h>

namespace UnTech {
namespace View {

// NOT RECOMMENDED FOR LARGE LISTS

template <class T>
class NamedListCtrl : public MyListCtrl {
    typedef typename Controller::NamedListController<T> NamedListController;

public:
    NamedListCtrl(wxWindow* parent, wxWindowID id,
                  NamedListController& controller)
        : MyListCtrl(parent, id, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL | wxLC_SINGLE_SEL)
        , _controller(controller)
        , _ptrList()
        , _nameList()
    {
        CreateColumns();

        /*
         * EVENTS
         * ======
         */
        this->Bind(wxEVT_LIST_ITEM_SELECTED, [this](wxCommandEvent&) {
            long i = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            if (i != wxNOT_FOUND && i >= 0) {
                unsigned id = i;
                if (id < _ptrList.size()) {
                    _controller.setSelected(_ptrList[i]);
                }
            }
            else {
                _controller.setSelected(nullptr);
            }
        });

        /*
         * SLOTS
         * =====
         */
        _controller.signal_listChanged().connect(sigc::mem_fun(
            *this, &NamedListCtrl::UpdateLists));

        // ::TODO get signal to emit the list not the item::
        _controller.signal_itemRenamed().connect(sigc::hide(sigc::mem_fun(
            *this, &NamedListCtrl::UpdateLists)));

        _controller.signal_listDataChanged().connect(
            [this](const typename T::list_t* list) {
                if (list == _controller.list()) {
                    UpdateLists();
                }
            });

        _controller.signal_selectedChanged().connect(
            [this](void) {
                const T* item = _controller.selected();

                for (unsigned i = 0; i < _ptrList.size(); i++) {
                    if (_ptrList[i] == item) {
                        SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
                        EnsureVisible(i);
                        return;
                    }
                }

                // not found - deselect the item(s)
                long i = -1;
                while ((i = GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED))
                       != wxNOT_FOUND) {
                    SetItemState(i, 0, wxLIST_STATE_SELECTED);
                }
            });

        _controller.signal_dataChanged().connect(
            [this](const T* item) {
                for (unsigned i = 0; i < _ptrList.size(); i++) {
                    if (_ptrList[i] == item) {
                        RefreshItem(i);
                        return;
                    }
                }
            });
    }

protected:
    // MUST BE IMPLEMENTED in a template specialisation
    // creates columns using `InsertColumn`
    virtual void CreateColumns();

    // MUST BE IMPLEMENTED in a template specialisation
    // Access item name/data with `_nameList[item]` & `_ptrList[item]`
    virtual wxString OnGetItemText(long item, long column) const override;

private:
    void UpdateLists()
    {
        const T* selected = _controller.selected();
        const typename T::list_t* list = _controller.list();

        if (list) {
            if (_ptrList.size() != list->size()) {
                SetItemCount(0);
                _ptrList.assign(list->size(), nullptr);
                _nameList.resize(list->size());
            }

            unsigned i = 0;
            optional<unsigned> indexOfSelected;
            for (const auto& it : *list) {
                _nameList[i] = it.first;
                _ptrList[i] = &it.second;

                if (selected == &it.second) {
                    indexOfSelected = i;
                }

                i++;
            }

            SetItemCount(list->size());
            Refresh();
            Enable();

            if (indexOfSelected) {
                SetItemState(indexOfSelected.value(),
                             wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            }
        }
        else {
            SetItemCount(0);
            _ptrList.clear();
            _nameList.clear();
            Disable();
        }
    }

private:
    NamedListController& _controller;
    std::vector<const T*> _ptrList;
    std::vector<wxString> _nameList;
};

template <class T>
class NamedListToolBar : public wxToolBar {
    typedef typename Controller::NamedListController<T> NamedListController;

    enum IDs {
        ID_CREATE = 1000,
        ID_CLONE,
        ID_RENAME,
        ID_REMOVE,
    };

public:
    NamedListToolBar(wxWindow* parent, wxWindowID id,
                     NamedListController& controller)
        : wxToolBar(parent, id, wxDefaultPosition, wxDefaultSize,
                    wxTB_HORIZONTAL | wxTB_NODIVIDER)
        , _controller(controller)
    {
        AddTool(ID_CREATE, "Create",
                wxArtProvider::GetBitmap("wxART_PLUS", wxART_TOOLBAR),
                wxString("Create ") + T::TYPE_NAME);

        AddTool(ID_CLONE, "Clone",
                wxArtProvider::GetBitmap("wxART_COPY", wxART_TOOLBAR),
                wxString("Clone ") + T::TYPE_NAME);

        AddTool(ID_RENAME, "Rename",
                wxArtProvider::GetBitmap("wxART_LIST_VIEW", wxART_TOOLBAR),
                wxString("Rename ") + T::TYPE_NAME);

        AddTool(ID_REMOVE, "Remove",
                wxArtProvider::GetBitmap("wxART_MINUS", wxART_TOOLBAR),
                wxString("Remove ") + T::TYPE_NAME);

        Realize();

        /*
         * EVENTS
         * ======
         */
        this->Bind(wxEVT_TOOL, [this](wxCommandEvent& e) {
            const typename T::list_t* list = _controller.list();

            if (list == nullptr) {
                return;
            }

            auto validator = [list](const std::string& name) {
                return isNameValid(name) && !list->nameExists(name);
            };

            // ::TODO create custom dialog for NameList Ids::

            switch (e.GetId()) {
            case ID_CREATE: {
                NamedListNameDialog dialog(this,
                                           wxString("Input ") + T::TYPE_NAME + " name",
                                           wxString("Input name of new ") + T::TYPE_NAME,
                                           validator);

                if (dialog.ShowModal() == wxID_OK) {
                    _controller.create(dialog.GetValue().ToStdString());
                }
                break;
            }

            case ID_CLONE: {
                NamedListNameDialog dialog(this,
                                           wxString("Input ") + T::TYPE_NAME + " name",
                                           wxString("Input new name of cloned ") + T::TYPE_NAME,
                                           validator);
                dialog.SetValue(_controller.selectedName());

                if (dialog.ShowModal() == wxID_OK) {
                    _controller.selected_clone(dialog.GetValue().ToStdString());
                }
                break;
            }

            case ID_RENAME: {
                NamedListNameDialog dialog(this,
                                           wxString("Input ") + T::TYPE_NAME + " name",
                                           wxString("Rename ") + T::TYPE_NAME + " to",
                                           validator);
                dialog.SetValue(_controller.selectedName());

                if (dialog.ShowModal() == wxID_OK) {
                    _controller.selected_rename(dialog.GetValue().ToStdString());
                }
                break;
            }

            case ID_REMOVE:
                _controller.selected_remove();
                break;
            }
        });

        /*
         * SIGNALS
         * =======
         */
        _controller.signal_listChanged().connect(sigc::mem_fun(
            *this, &NamedListToolBar::UpdateGui));

        _controller.signal_selectedChanged().connect(sigc::mem_fun(
            *this, &NamedListToolBar::UpdateGui));
    }

protected:
    void UpdateGui()
    {
        const T* item = _controller.selected();
        const typename T::list_t* list = _controller.list();

        EnableTool(ID_CREATE, list != nullptr);

        EnableTool(ID_CLONE, item != nullptr);
        EnableTool(ID_RENAME, item != nullptr);
        EnableTool(ID_REMOVE, item != nullptr);
    }

private:
    NamedListController& _controller;
};
}
}
