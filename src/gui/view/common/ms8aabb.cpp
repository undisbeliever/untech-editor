#include "ms8aabb.h"
#include "gui/view/defaults.h"

#include <cstdint>
#include <wx/wrapsizer.h>

using namespace UnTech;
using namespace UnTech::View;

// BUGFIX:
// for some reason the spin controls are not equal in width unless I set
// wxSpinCtrl::SetSizeHints to a positive value.
static const int MIN_SPIN_WIDTH = 80;

Ms8PointCtrl::Ms8PointCtrl(wxWindow* parent, int wxWindowID)
    : wxPanel(parent, wxWindowID)
{
    const wxSize commaSize(AABB_COMMA_WIDTH, -1);

    auto* sizer = new wxBoxSizer(wxHORIZONTAL);

    _xPos = new wxSpinCtrl(this, wxID_ANY);
    _xPos->SetRange(int_ms8_t::MIN, int_ms8_t::MAX);
    _xPos->SetSizeHints(MIN_SPIN_WIDTH, -1);
    sizer->Add(_xPos, 1, wxEXPAND);

    auto* comma = new wxStaticText(this, wxID_ANY, ",",
                                   wxDefaultPosition, commaSize,
                                   wxALIGN_CENTER_HORIZONTAL | wxST_NO_AUTORESIZE);
    sizer->Add(comma, 0, wxALIGN_BOTTOM | wxALIGN_CENTER_HORIZONTAL | wxALL, 1);

    _yPos = new wxSpinCtrl(this, wxID_ANY);
    _yPos->SetRange(int_ms8_t::MIN, int_ms8_t::MAX);
    _yPos->SetSizeHints(MIN_SPIN_WIDTH, -1);
    sizer->Add(_yPos, 1, wxEXPAND);

    this->SetSizer(sizer);
}

Ms8RectCtrl::Ms8RectCtrl(wxWindow* parent, int wxWindowID)
    : wxPanel(parent, wxWindowID)
{
    const wxSize commaSize(AABB_COMMA_WIDTH, -1);

    auto* sizer = new wxBoxSizer(wxVERTICAL);

    auto* posSizer = new wxBoxSizer(wxHORIZONTAL);

    _xPos = new wxSpinCtrl(this, wxID_ANY);
    _xPos->SetRange(int_ms8_t::MIN, int_ms8_t::MAX);
    _xPos->SetSizeHints(MIN_SPIN_WIDTH, -1);
    posSizer->Add(_xPos, 1, wxEXPAND);

    auto* comma = new wxStaticText(this, wxID_ANY, ",",
                                   wxDefaultPosition, commaSize,
                                   wxALIGN_CENTER_HORIZONTAL | wxST_NO_AUTORESIZE);
    posSizer->Add(comma, 0, wxALIGN_BOTTOM | wxALIGN_CENTER_HORIZONTAL | wxALL, 1);

    _yPos = new wxSpinCtrl(this, wxID_ANY);
    _yPos->SetRange(int_ms8_t::MIN, int_ms8_t::MAX);
    _yPos->SetSizeHints(MIN_SPIN_WIDTH, -1);
    posSizer->Add(_yPos, 1, wxEXPAND);

    sizer->Add(posSizer, 1, wxEXPAND);

    sizer->AddSpacer(DEFAULT_HGAP);

    auto* sizeSizer = new wxBoxSizer(wxHORIZONTAL);

    _width = new wxSpinCtrl(this, wxID_ANY);
    _width->SetRange(0, UINT8_MAX);
    _width->SetSizeHints(MIN_SPIN_WIDTH, -1);
    sizeSizer->Add(_width, 1, wxEXPAND, 0);

    auto* cross = new wxStaticText(this, wxID_ANY, "x",
                                   wxDefaultPosition, commaSize,
                                   wxALIGN_CENTER_HORIZONTAL | wxST_NO_AUTORESIZE);
    sizeSizer->Add(cross, 0, wxALIGN_BOTTOM | wxALIGN_CENTER_HORIZONTAL | wxALL, 1);

    _height = new wxSpinCtrl(this, wxID_ANY);
    _height->SetRange(0, UINT8_MAX);
    _height->SetSizeHints(MIN_SPIN_WIDTH, -1);
    sizeSizer->Add(_height, 1, wxEXPAND);

    sizer->Add(sizeSizer, 1, wxEXPAND);

    this->SetSizer(sizer);
}
