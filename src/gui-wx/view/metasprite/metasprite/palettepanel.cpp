/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "palettepanel.h"
#include "editcolordialog.h"
#include "gui-wx/view/defaults.h"

using namespace UnTech::View::MetaSprite::MetaSprite;

PalettePanel::PalettePanel(wxWindow* parent, int wxWindowID,
                           MS::PaletteController& controller)

    : wxPanel(parent, wxWindowID)
    , _controller(controller)
{
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(sizer);

    _editColor = new wxToggleButton(this, wxID_ANY, "Edit");
    sizer->Add(_editColor, wxSizerFlags(0).Right().Border(wxBOTTOM));

    auto* grid = new wxGridSizer(2, 8, 1, 1);
    sizer->Add(grid, wxSizerFlags(0).Center());

    wxSize buttonSize(SIDEBAR_WIDTH / 9, SIDEBAR_WIDTH / 9);

    for (unsigned i = 0; i < N_COLORS; i++) {
        _colors[i] = new wxToggleButton(this, ID_COLOR_0 + i, "",
                                        wxDefaultPosition, buttonSize);

        grid->Add(_colors[i], 1);
    }

    // Signals
    // -------
    _controller.signal_anyChanged().connect(sigc::mem_fun(
        *this, &PalettePanel::UpdateGui));

    _controller.signal_selectedColorChanged().connect(sigc::mem_fun(
        *this, &PalettePanel::UpdateGui));

    // EVENTS
    // ------
    _editColor->Bind(wxEVT_TOGGLEBUTTON, &PalettePanel::on_editColorToggled, this);

    this->Bind(wxEVT_TOGGLEBUTTON, &PalettePanel::on_colorToggled, this, ID_COLOR_0, ID_COLOR_15);
}

void PalettePanel::UpdateGui()
{
    if (_controller.hasSelected()) {
        const Snes::Palette4bpp& palette = _controller.selected();
        int active = _controller.selectedColorId();

        if (active > 0) {
            _editColor->SetValue(false);
        }

        for (unsigned i = 0; i < N_COLORS; i++) {
            _colors[i]->SetBackgroundColour(wxColor(palette.color(i).rgb().rgb()));
            _colors[i]->SetValue((int)i == active);
        }

        this->Enable();
    }
    else {
        _editColor->SetValue(false);
        for (unsigned i = 0; i < N_COLORS; i++) {
            _colors[i]->SetBackgroundColour(wxNullColour);
            _colors[i]->SetValue(false);
        }

        this->Disable();
    }
}

void PalettePanel::on_colorToggled(wxCommandEvent& e)
{
    unsigned c = e.GetId() - ID_COLOR_0;

    // unselect all other colors
    for (unsigned i = 0; i < N_COLORS; i++) {
        if (i != c) {
            _colors[i]->SetValue(false);
        }
    }

    if (_editColor->GetValue() == false) {
        // select color
        if (_colors[c]->GetValue()) {
            _controller.setSelectedColorId(c);
        }
        else {
            _controller.setSelectedColorId(-1);
        }
    }
    else {
        // edit color
        if (_colors[c]->GetValue() == true) {
            EditColorDialog dialog(this, _controller, c);
            dialog.ShowModal();

            _colors[c]->SetValue(false);
        }
    }
}

void PalettePanel::on_editColorToggled(wxCommandEvent&)
{
    // unselect all colors
    for (unsigned i = 0; i < N_COLORS; i++) {
        _colors[i]->SetValue(false);
    }

    _controller.setSelectedColorId(-1);
}
