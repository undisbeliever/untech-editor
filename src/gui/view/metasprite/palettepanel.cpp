#include "palettepanel.h"
#include "gui/view/defaults.h"

using namespace UnTech::View::MetaSprite;

PalettePanel::PalettePanel(wxWindow* parent, int wxWindowID)

    : wxPanel(parent, wxWindowID)
{
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(sizer);

    _editColor = new wxToggleButton(this, wxID_ANY, "Edit");
    sizer->Add(_editColor, 0, wxRIGHT | wxALIGN_RIGHT, DEFAULT_BORDER);

    auto* grid = new wxGridSizer(2, 8, 0, 0);
    sizer->Add(grid, 0, wxEXPAND | wxTOP, DEFAULT_BORDER);

    wxSize buttonSize(SIDEBAR_WIDTH / 9, SIDEBAR_WIDTH / 9);

    for (unsigned i = 0; i < N_COLORS; i++) {
        _colors[i] = new wxToggleButton(this, ID_COLOR_0 + i, "", wxDefaultPosition, buttonSize);

        _colors[i]->Bind(wxEVT_TOGGLEBUTTON, &PalettePanel::on_colorToggled, this);

        grid->Add(_colors[i], 1);
    }

    // EVENTS
    // ======
    _editColor->Bind(wxEVT_TOGGLEBUTTON, &PalettePanel::on_editColorToggled, this);
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
}

void PalettePanel::on_editColorToggled(wxCommandEvent&)
{
    // unselect all colors
    for (unsigned i = 0; i < N_COLORS; i++) {
        _colors[i]->SetValue(false);
    }
}
