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
    sizer->Add(_xPos, wxSizerFlags(1).Expand());

    auto* comma = new wxStaticText(this, wxID_ANY, ",",
                                   wxDefaultPosition, commaSize,
                                   wxALIGN_CENTER_HORIZONTAL | wxST_NO_AUTORESIZE);
    sizer->Add(comma, wxSizerFlags().Bottom().Border(wxALL, 1));

    _yPos = new wxSpinCtrl(this, wxID_ANY);
    _yPos->SetRange(int_ms8_t::MIN, int_ms8_t::MAX);
    _yPos->SetSizeHints(MIN_SPIN_WIDTH, -1);
    sizer->Add(_yPos, wxSizerFlags(1).Expand());

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
    posSizer->Add(_xPos, wxSizerFlags(1).Expand());

    auto* comma = new wxStaticText(this, wxID_ANY, ",",
                                   wxDefaultPosition, commaSize,
                                   wxALIGN_CENTER_HORIZONTAL | wxST_NO_AUTORESIZE);
    posSizer->Add(comma, wxSizerFlags(0).Bottom().Border(wxALL, 1));

    _yPos = new wxSpinCtrl(this, wxID_ANY);
    _yPos->SetRange(int_ms8_t::MIN, int_ms8_t::MAX);
    _yPos->SetSizeHints(MIN_SPIN_WIDTH, -1);
    posSizer->Add(_yPos, wxSizerFlags(1).Expand());

    sizer->Add(posSizer, wxSizerFlags(1).Expand());

    sizer->AddSpacer(wxSizerFlags::GetDefaultBorder());

    auto* sizeSizer = new wxBoxSizer(wxHORIZONTAL);

    _width = new wxSpinCtrl(this, wxID_ANY);
    _width->SetRange(0, UINT8_MAX);
    _width->SetSizeHints(MIN_SPIN_WIDTH, -1);
    sizeSizer->Add(_width, wxSizerFlags(1).Expand());

    auto* cross = new wxStaticText(this, wxID_ANY, "x",
                                   wxDefaultPosition, commaSize,
                                   wxALIGN_CENTER_HORIZONTAL | wxST_NO_AUTORESIZE);
    sizeSizer->Add(cross, wxSizerFlags(0).Bottom().Border(wxALL, 1));

    _height = new wxSpinCtrl(this, wxID_ANY);
    _height->SetRange(0, UINT8_MAX);
    _height->SetSizeHints(MIN_SPIN_WIDTH, -1);
    sizeSizer->Add(_height, wxSizerFlags(1).Expand());

    sizer->Add(sizeSizer, wxSizerFlags(1).Expand());

    this->SetSizer(sizer);
}
