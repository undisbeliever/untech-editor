/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once
#include "mylistctrl.h"
#include "gui-wx/view/defaults.h"
#include "models/common/humantypename.h"
#include <cassert>
#include <sigc++/signal.h>
#include <wx/artprov.h>
#include <wx/listctrl.h>
#include <wx/toolbar.h>
#include <wx/wx.h>

namespace UnTech {
namespace View {

template <class ControllerT>
class VectorListCtrl : public MyListCtrl {
public:
    using controller_type = ControllerT;
    using list_type = typename ControllerT::list_type;
    using element_type = typename ControllerT::element_type;

    VectorListCtrl(wxWindow* parent, wxWindowID id,
                   controller_type& controller)
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
                _controller.selectIndex(i);
            }
            else {
                _controller.selectNone();
            }
        });

        /*
         * SLOTS
         * =====
         */

        _controller.signal_listChanged().connect(
            [this](void) {
                const list_type* list = _controller.list();
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
                const list_type* list = _controller.list();
                optional<size_t> index = _controller.selectedIndex();

                if (list && index) {
                    SetItemCount(list->size()); // BUGFIX
                    SetItemState(index.value(), wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
                    EnsureVisible(index.value());
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

        _controller.signal_dataChanged().connect([this](void) {
            this->Refresh();
        });
    }

protected:
    // MUST BE IMPLEMENTED in a template specialisation
    // creates columns using `InsertColumn`
    virtual void CreateColumns();

    // MUST BE IMPLEMENTED in a template specialisation
    virtual wxString OnGetItemText(long item, long column) const override;

private:
    controller_type& _controller;
};

template <class ControllerT>
class VectorListToolBar : public wxToolBar {
    using controller_type = ControllerT;

    enum IDs {
        ID_CREATE = 1000,
        ID_CLONE,
        ID_MOVE_UP,
        ID_MOVE_DOWN,
        ID_REMOVE,
    };

public:
    VectorListToolBar(wxWindow* parent, wxWindowID id,
                      controller_type& controller)
        : wxToolBar(parent, id, wxDefaultPosition, wxDefaultSize,
                    wxTB_HORIZONTAL | wxTB_NODIVIDER)
        , _controller(controller)
    {
        const static std::string& HUMAN_TYPE_NAME = HumanTypeName<typename controller_type::element_type>::value;

        AddTool(ID_CREATE, "Create",
                wxArtProvider::GetBitmap("wxART_PLUS", wxART_TOOLBAR),
                wxString("Create ") + HUMAN_TYPE_NAME);

        AddTool(ID_CLONE, "Clone",
                wxArtProvider::GetBitmap("wxART_COPY", wxART_TOOLBAR),
                wxString("Clone ") + HUMAN_TYPE_NAME);

        AddTool(ID_MOVE_UP, "Move Up",
                wxArtProvider::GetBitmap("wxART_GO_UP", wxART_TOOLBAR),
                wxString("Move ") + HUMAN_TYPE_NAME + " Up");

        AddTool(ID_MOVE_DOWN, "Move Down",
                wxArtProvider::GetBitmap("wxART_GO_DOWN", wxART_TOOLBAR),
                wxString("Move ") + HUMAN_TYPE_NAME + " Down");

        AddTool(ID_REMOVE, "Remove",
                wxArtProvider::GetBitmap("wxART_MINUS", wxART_TOOLBAR),
                wxString("Remove ") + HUMAN_TYPE_NAME);

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
                _controller.cloneSelected();
                break;

            case ID_MOVE_UP:
                _controller.moveSelectedUp();
                UpdateGui();
                break;

            case ID_MOVE_DOWN:
                _controller.moveSelectedDown();
                UpdateGui();
                break;

            case ID_REMOVE:
                _controller.removeSelected();
                break;
            }
        });

        /*
         * SIGNALS
         * =======
         */
        _controller.signal_listChanged().connect(sigc::mem_fun(
            *this, &VectorListToolBar::UpdateGui));

        _controller.signal_selectedChanged().connect(sigc::mem_fun(
            *this, &VectorListToolBar::UpdateGui));
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
    controller_type& _controller;
};
}
}
