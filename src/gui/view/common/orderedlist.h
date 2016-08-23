#pragma once
#include "gui/controllers/helpers/orderedlistcontroller.h"
#include "gui/view/defaults.h"
#include <cassert>
#include <wx/artprov.h>
#include <wx/listctrl.h>
#include <wx/toolbar.h>
#include <wx/wx.h>

namespace UnTech {
namespace View {

template <class T>
class OrderedListCtrl : public MyListCtrl {
    typedef typename Controller::OrderedListController<T> OrderedListController;

public:
    OrderedListCtrl(wxWindow* parent, wxWindowID id,
                    OrderedListController& controller)
        : MyListCtrl(parent, id, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL | wxLC_SINGLE_SEL)
        , _controller(controller)
    {
        CreateColumns();

        /*
         * EVENTS
         * ======
         */
        this->Bind(wxEVT_LIST_ITEM_SELECTED, [this](wxCommandEvent&) {
            long i = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            if (i != wxNOT_FOUND) {
                _controller.setSelected(i);
            }
            else {
                _controller.setSelected(nullptr);
            }
        });

        /*
         * SLOTS
         * =====
         */
        _controller.signal_listChanged().connect(
            [this](void) {
                const typename T::list_t* list = _controller.list();
                if (list) {
                    SetItemCount(list->size());
                    Refresh();
                    Enable();
                }
                else {
                    SetItemCount(0);
                    Disable();
                }
            });

        _controller.signal_selectedChanged().connect(
            [this](void) {
                const T* item = _controller.selected();
                const typename T::list_t* list = _controller.list();
                if (list && item) {
                    int i = list->indexOf(item);
                    if (i >= 0) {
                        SetItemCount(list->size()); // BUGFIX
                        SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
                        EnsureVisible(i);
                    }
                }
                else {
                    // deselect the item(s)
                    long i = -1;
                    while ((i = GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED))
                           != wxNOT_FOUND) {
                        SetItemState(i, 0, wxLIST_STATE_SELECTED);
                    }
                }
            });

        _controller.signal_listDataChanged().connect(
            [this](const typename T::list_t* list) {
                if (list && list == _controller.list()) {
                    SetItemCount(list->size());

                    const T* item = _controller.selected();
                    if (item) {
                        int i = list->indexOf(item);
                        if (i >= 0) {
                            SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
                            EnsureVisible(i);
                        }
                    }

                    Refresh();
                }
            });

        _controller.signal_dataChanged().connect(
            [this](const T* item) {
                const typename T::list_t* list = _controller.list();
                if (item && list) {
                    int i = list->indexOf(item);
                    if (i >= 0) {
                        RefreshItem(i);
                    }
                }
            });
    }

protected:
    // MUST BE IMPLEMENTED in a template specialisation
    // creates columns using `InsertColumn`
    virtual void CreateColumns();

    // MUST BE IMPLEMENTED in a template specialisation
    virtual wxString OnGetItemText(long item, long column) const override;

private:
    OrderedListController& _controller;
};

template <class T>
class OrderedListToolBar : public wxToolBar {
    typedef typename Controller::OrderedListController<T> OrderedListController;

    enum IDs {
        ID_CREATE = 1000,
        ID_CLONE,
        ID_MOVE_UP,
        ID_MOVE_DOWN,
        ID_REMOVE,
    };

public:
    OrderedListToolBar(wxWindow* parent, wxWindowID id,
                       OrderedListController& controller)
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

        AddTool(ID_MOVE_UP, "Move Up",
                wxArtProvider::GetBitmap("wxART_GO_UP", wxART_TOOLBAR),
                wxString("Move ") + T::TYPE_NAME + " Up");

        AddTool(ID_MOVE_DOWN, "Move Down",
                wxArtProvider::GetBitmap("wxART_GO_DOWN", wxART_TOOLBAR),
                wxString("Move ") + T::TYPE_NAME + " Down");

        AddTool(ID_REMOVE, "Remove",
                wxArtProvider::GetBitmap("wxART_MINUS", wxART_TOOLBAR),
                wxString("Remove ") + T::TYPE_NAME);

        Realize();

        /*
         * EVENTS
         * ======
         */
        this->Bind(wxEVT_TOOL, [this](wxCommandEvent& e) {
            switch (e.GetId()) {
            case ID_CREATE:
                _controller.create();
                break;

            case ID_CLONE:
                _controller.selected_clone();
                break;

            case ID_MOVE_UP:
                _controller.selected_moveUp();
                UpdateGui();
                break;

            case ID_MOVE_DOWN:
                _controller.selected_moveDown();
                UpdateGui();
                break;

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
            *this, &OrderedListToolBar::UpdateGui));

        _controller.signal_selectedChanged().connect(sigc::mem_fun(
            *this, &OrderedListToolBar::UpdateGui));
    }

protected:
    void UpdateGui()
    {
        bool hasList = _controller.list() != nullptr;
        this->Enable(hasList);

        if (hasList) {
            EnableTool(ID_CREATE, _controller.canCreate());
            EnableTool(ID_CLONE, _controller.canCloneSelected());
            EnableTool(ID_MOVE_UP, _controller.canMoveSelectedUp());
            EnableTool(ID_MOVE_DOWN, _controller.canMoveSelectedDown());
            EnableTool(ID_REMOVE, _controller.canRemoveSelected());
        }
    }

private:
    OrderedListController& _controller;
};
}
}
