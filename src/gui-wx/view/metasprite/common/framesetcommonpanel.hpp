/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once
#include "gui-wx/view/common/enumclasschoice.h"
#include "gui-wx/view/common/filedialogs.h"
#include "gui-wx/view/common/idstringtextctrl.h"
#include "gui-wx/view/common/textandtogglebuttonctrl.h"
#include "gui-wx/view/defaults.h"
#include "models/metasprite/frameset-exportorder.h"
#include "models/metasprite/tilesettype.h"
#include <sigc++/signal.h>
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSprite {
namespace Common {

template <class FsControllerT>
class FrameSetCommonPanel : public wxPanel {
public:
    FrameSetCommonPanel(wxWindow* parent, int wxWindowID,
                        FsControllerT& controller)
        : wxPanel(parent, wxWindowID)
        , _controller(controller)
    {
        int defBorder = wxSizerFlags::GetDefaultBorder();
        auto* grid = new wxFlexGridSizer(4, 2, defBorder, defBorder * 2);
        this->SetSizer(grid);

        grid->AddGrowableCol(1, 1);

        _name = new IdStringTextCtrl(this, wxID_ANY,
                                     idstring(), wxDefaultPosition, wxDefaultSize,
                                     wxTE_PROCESS_ENTER);
        grid->Add(new wxStaticText(this, wxID_ANY, wxT("Name:")));
        grid->Add(_name, wxSizerFlags().Expand());

        _tilesetType = new EnumClassChoice<UnTech::MetaSprite::TilesetType>(this, wxID_ANY);
        grid->Add(new wxStaticText(this, wxID_ANY, wxT("Tileset Type:")));
        grid->Add(_tilesetType, wxSizerFlags().Expand());

        grid->Add(new wxStaticText(this, wxID_ANY, wxT("Export Order:")));
        _exportOrder = new TextAndToggleButtonCtrl(this, wxID_ANY);
        grid->Add(_exportOrder, wxSizerFlags().Expand());

        _frameSetType = new wxTextCtrl(this, wxID_ANY);
        _frameSetType->Disable();
        grid->Add(new wxStaticText(this, wxID_ANY, wxT("FrameSet Type:")));
        grid->Add(_frameSetType, wxSizerFlags().Expand());

        // Signals
        // -------
        _controller.signal_selectedChanged().connect(sigc::mem_fun(
            *this, &FrameSetCommonPanel::UpdateGui));

        _controller.signal_dataChanged().connect(sigc::mem_fun(
            *this, &FrameSetCommonPanel::UpdateGui));

        // Events
        // ------
        _name->Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent&) {
            _controller.selected_setName(_name->GetValue().ToStdString());
            this->NavigateIn();
        });
        _name->Bind(wxEVT_KILL_FOCUS, [this](wxFocusEvent& e) {
            _controller.selected_setName(_name->GetValue().ToStdString());
            e.Skip();
        });

        _tilesetType->Bind(wxEVT_CHOICE, [this](wxCommandEvent&) {
            _controller.selected_setTilesetType(_tilesetType->GetValue());
        });

        _exportOrder->Bind(wxEVT_TOGGLEBUTTON, [this](wxCommandEvent&) {
            using FSEO = UnTech::MetaSprite::FrameSetExportOrder;

            const auto& frameSet = _controller.selected();

            if (_exportOrder->GetButtonValue()) {
                const static DocumentType documentType = { "Frame Set Export File", FSEO::FILE_EXTENSION };

                std::string oldFileName;
                if (frameSet.exportOrder != nullptr) {
                    oldFileName = frameSet.exportOrder->filename;
                }

                auto fn = openFileDialog(this, documentType, oldFileName);

                if (fn) {
                    _controller.selected_setExportOrderFilename(fn.value());
                }

                _exportOrder->SetButtonValue(false);
            }
        });
    }

    void UpdateGui()
    {
        const auto& frameSet = _controller.selected();

        this->Enable(_controller.hasSelected());

        _name->ChangeValue(frameSet.name.str());
        _tilesetType->SetValue(frameSet.tilesetType);

        const auto& fseo = frameSet.exportOrder;
        if (fseo) {
            _exportOrder->ChangeTextValue(fseo->filename);
            _frameSetType->ChangeValue(fseo->name.str());
        }
        else {
            _exportOrder->ChangeTextValue(wxEmptyString);
            _frameSetType->ChangeValue(wxEmptyString);
        }
    }

private:
    FsControllerT& _controller;

    IdStringTextCtrl* _name;
    EnumClassChoice<UnTech::MetaSprite::TilesetType>* _tilesetType;
    TextAndToggleButtonCtrl* _exportOrder;
    wxTextCtrl* _frameSetType;
};
}
}
}
}
