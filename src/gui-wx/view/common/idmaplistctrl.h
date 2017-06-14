/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "idstringdialog.h"
#include "mylistctrl.h"
#include "gui-wx/controllers/containers/idmapcontroller.h"
#include "gui-wx/view/defaults.h"
#include "models/common/humantypename.h"
#include "models/common/optional.h"
#include <cassert>
#include <sigc++/sigc++.h>
#include <vector>
#include <wx/artprov.h>
#include <wx/listctrl.h>
#include <wx/toolbar.h>
#include <wx/wx.h>

namespace UnTech {
namespace View {

// NOT RECOMMENDED FOR LARGE LISTS

template <class ControllerT>
class IdMapListCtrl : public MyListCtrl {
public:
    using controller_type = ControllerT;
    using map_type = typename ControllerT::map_type;
    using element_type = typename ControllerT::element_type;

    IdMapListCtrl(wxWindow* parent, wxWindowID id,
                  controller_type& controller)
        : MyListCtrl(parent, id, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL | wxLC_SINGLE_SEL)
        , _controller(controller)
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
                _controller.selectId(_nameList[i].ToStdString());
            }
            else {
                _controller.selectNone();
            }
        });

        /*
         * SLOTS
         * =====
         */

        _controller.signal_mapChanged().connect(sigc::mem_fun(
            *this, &IdMapListCtrl::UpdateLists));

        _controller.signal_selectedChanged().connect(
            [this](void) {
                const idstring& id = _controller.selectedId();

                if (id.isValid()) {
                    for (unsigned i = 0; i < _nameList.size(); i++) {
                        if (_nameList[i] == id.str()) {
                            SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
                            EnsureVisible(i);
                            return;
                        }
                    }
                }

                // not found - deselect the item(s)
                long i = -1;
                while ((i = GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED))
                       != wxNOT_FOUND) {
                    SetItemState(i, 0, wxLIST_STATE_SELECTED);
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
    // Access item name/data with `_nameList[item]` & `_ptrList[item]`
    virtual wxString OnGetItemText(long item, long column) const override;

private:
    void UpdateLists()
    {
        const idstring& selectedId = _controller.selectedId();
        const map_type* map = _controller.map();

        if (map) {
            if (_nameList.size() != map->size()) {
                SetItemCount(0);
                _nameList.resize(map->size());
            }

            unsigned i = 0;
            optional<unsigned> indexOfSelected;
            for (const auto& it : *map) {
                _nameList[i] = it.first.str();

                if (selectedId == it.first) {
                    indexOfSelected = i;
                }

                i++;
            }

            SetItemCount(map->size());
            Refresh();
            Enable();

            if (indexOfSelected) {
                SetItemState(indexOfSelected.value(),
                             wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            }
        }
        else {
            SetItemCount(0);
            _nameList.clear();
            Disable();
        }
    }

private:
    controller_type& _controller;
    std::vector<wxString> _nameList;
};

template <class ControllerT>
class IdMapListToolBar : public wxToolBar {
    using controller_type = ControllerT;
    using map_type = typename ControllerT::map_type;

    enum IDs {
        ID_CREATE = 1000,
        ID_CLONE,
        ID_RENAME,
        ID_REMOVE,
    };

public:
    IdMapListToolBar(wxWindow* parent, wxWindowID id,
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

        AddTool(ID_RENAME, "Rename",
                wxArtProvider::GetBitmap("wxART_LIST_VIEW", wxART_TOOLBAR),
                wxString("Rename ") + HUMAN_TYPE_NAME);

        AddTool(ID_REMOVE, "Remove",
                wxArtProvider::GetBitmap("wxART_MINUS", wxART_TOOLBAR),
                wxString("Remove ") + HUMAN_TYPE_NAME);

        Realize();

        /*
         * EVENTS
         * ======
         */
        this->Bind(wxEVT_TOOL, [this](wxCommandEvent& e) {
            const map_type* map = _controller.map();

            if (map == nullptr) {
                return;
            }

            auto validator = [map](const idstring& id) {
                return id.isValid() && !map->contains(id);
            };

            const static std::string& HUMAN_TYPE_NAME = HumanTypeName<typename controller_type::element_type>::value;

            switch (e.GetId()) {
            case ID_CREATE: {
                IdStringDialog dialog(this,
                                      wxString("Input ") + HUMAN_TYPE_NAME + " name",
                                      wxString("Input name of new ") + HUMAN_TYPE_NAME,
                                      validator);

                if (dialog.ShowModal() == wxID_OK) {
                    _controller.create(dialog.GetIdString());
                }
                break;
            }

            case ID_CLONE: {
                IdStringDialog dialog(this,
                                      wxString("Input ") + HUMAN_TYPE_NAME + " name",
                                      wxString("Input new name of cloned ") + HUMAN_TYPE_NAME,
                                      validator);
                dialog.SetIdString(_controller.selectedId());

                if (dialog.ShowModal() == wxID_OK) {
                    _controller.cloneSelected(dialog.GetIdString());
                }
                break;
            }

            case ID_RENAME: {
                IdStringDialog dialog(this,
                                      wxString("Input ") + HUMAN_TYPE_NAME + " name",
                                      wxString("Rename ") + HUMAN_TYPE_NAME + " to",
                                      validator);
                dialog.SetIdString(_controller.selectedId());

                if (dialog.ShowModal() == wxID_OK) {
                    _controller.renameSelected(dialog.GetIdString());
                }
                break;
            }

            case ID_REMOVE:
                _controller.removeSelected();
                break;
            }
        });

        /*
         * SIGNALS
         * =======
         */
        _controller.signal_mapChanged().connect(sigc::mem_fun(
            *this, &IdMapListToolBar::UpdateGui));

        _controller.signal_selectedChanged().connect(sigc::mem_fun(
            *this, &IdMapListToolBar::UpdateGui));
    }

protected:
    void UpdateGui()
    {
        bool hasMap = _controller.map() != nullptr;
        this->Enable(hasMap);

        if (hasMap) {
            EnableTool(ID_CREATE, _controller.canCreate());
            EnableTool(ID_CLONE, _controller.canCloneSelected());
            EnableTool(ID_RENAME, _controller.canRenameSelected());
            EnableTool(ID_REMOVE, _controller.canRemoveSelected());
        }
    }

private:
    controller_type& _controller;
};
}
}
