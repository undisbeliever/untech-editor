#include "palettepanel.h"
#include "gui/view/defaults.h"

using namespace UnTech::View::MetaSprite;

PalettePanel::PalettePanel(wxWindow* parent, int wxWindowID,
                           MS::PaletteController& controller)

    : wxPanel(parent, wxWindowID)
    , _controller(controller)
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

    updateGui();

    // EVENTS
    // ======
    _editColor->Bind(wxEVT_TOGGLEBUTTON, &PalettePanel::on_editColorToggled, this);
}

void PalettePanel::updateGui()
{
    const MS::Palette* palette = _controller.selected();

    if (palette) {
        int active = _controller.selectedColorId();

        if (active > 0) {
            _editColor->SetValue(false);
        }

        for (unsigned i = 0; i < N_COLORS; i++) {
            _colors[i]->SetBackgroundColour(wxColor(palette->color(i).rgb().value));
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
}

void PalettePanel::on_editColorToggled(wxCommandEvent&)
{
    // unselect all colors
    for (unsigned i = 0; i < N_COLORS; i++) {
        _colors[i]->SetValue(false);
    }
}
