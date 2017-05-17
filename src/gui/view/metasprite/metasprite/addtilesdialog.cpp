/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "addtilesdialog.h"
#include "gui/view/defaults.h"

using namespace UnTech::View::MetaSprite::MetaSprite;

AddTilesDialog::AddTilesDialog(wxWindow* parent,
                               unsigned nSmall, unsigned nLarge)
    : wxDialog(parent, wxID_ANY, "Add Tiles",
               wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxCAPTION | wxCENTRE)
    , _nSmall(nSmall)
    , _nLarge(nLarge)
{
    // layout and borders match that of wxWidgets' wxTextEntryDialog

    auto* sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(sizer);

    sizer->Add(CreateTextSizer("Add tiles to FrameSet"),
               wxSizerFlags().DoubleBorder());

    int defBorder = wxSizerFlags::GetDefaultBorder();
    auto* grid = new wxGridSizer(2, 3, defBorder, defBorder * 2);
    sizer->Add(grid, wxSizerFlags().Expand().DoubleBorder(wxLEFT | wxRIGHT | wxBOTTOM));

    _addSmall = new wxSpinCtrl(this, wxID_ANY,
                               wxEmptyString, wxDefaultPosition, wxDefaultSize,
                               wxTE_PROCESS_ENTER);
    _addSmall->SetRange(0, 32);

    _smallTotal = new wxStaticText(this, wxID_ANY, wxEmptyString);

    grid->Add(new wxStaticText(this, wxID_ANY, "Small Tiles:"));
    grid->Add(_addSmall);
    grid->Add(_smallTotal);

    _addLarge = new wxSpinCtrl(this, wxID_ANY,
                               wxEmptyString, wxDefaultPosition, wxDefaultSize,
                               wxTE_PROCESS_ENTER);
    _addLarge->SetRange(0, 32);

    _largeTotal = new wxStaticText(this, wxID_ANY, wxEmptyString);

    grid->Add(new wxStaticText(this, wxID_ANY, "Large Tiles:"));
    grid->Add(_addLarge);
    grid->Add(_largeTotal);

    sizer->Add(CreateSeparatedButtonSizer(wxOK | wxCANCEL | wxNO_DEFAULT),
               wxSizerFlags().Expand().DoubleBorder());

    SetAutoLayout(true);
    sizer->Fit(this);

    UpdateGui();

    // EVENTS
    // ======
    this->Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent&) {
        UpdateGui();
        this->NavigateIn();
    });
    this->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        UpdateGui();
    });
}

void AddTilesDialog::UpdateGui()
{
    _smallTotal->SetLabel(wxString::Format("%d Total", _nSmall + _addSmall->GetValue()));
    _largeTotal->SetLabel(wxString::Format("%d Total", _nLarge + _addLarge->GetValue()));
}
