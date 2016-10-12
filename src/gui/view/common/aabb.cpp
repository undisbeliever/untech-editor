#include "aabb.h"
#include "gui/view/defaults.h"

#include <algorithm>
#include <cstdint>

using namespace UnTech;
using namespace UnTech::View;

// BUGFIX:
// for some reason the spin controls are not equal in width unless I set
// wxSpinCtrl::SetSizeHints to a positive value.
static const int MIN_SPIN_WIDTH = 80;

UPointCtrl::UPointCtrl(wxWindow* parent, int wxWindowID)
    : wxPanel(parent, wxWindowID)
{
    const wxSize commaSize(AABB_COMMA_WIDTH, -1);

    auto* sizer = new wxBoxSizer(wxHORIZONTAL);

    _xPos = new wxSpinCtrl(this, wxID_ANY);
    _xPos->SetRange(0, INT_MAX);
    _xPos->SetSizeHints(MIN_SPIN_WIDTH, -1);
    sizer->Add(_xPos, wxSizerFlags(1).Expand());

    auto* comma = new wxStaticText(this, wxID_ANY, ",",
                                   wxDefaultPosition, commaSize,
                                   wxALIGN_CENTER_HORIZONTAL | wxST_NO_AUTORESIZE);
    sizer->Add(comma, wxSizerFlags().Bottom().Border(wxALL, 1));

    _yPos = new wxSpinCtrl(this, wxID_ANY);
    _yPos->SetRange(0, INT_MAX);
    _yPos->SetSizeHints(MIN_SPIN_WIDTH, -1);
    sizer->Add(_yPos, wxSizerFlags(1).Expand());

    this->SetSizer(sizer);
}

void UPointCtrl::SetRange(const usize& range)
{
    _xPos->SetRange(0, range.width - 1);
    _yPos->SetRange(0, range.height - 1);
}

void UPointCtrl::SetRange(const usize& range, unsigned squareSize)
{
    _xPos->SetRange(0, std::max((int)(range.width - squareSize), 0));
    _yPos->SetRange(0, std::max((int)(range.height - squareSize), 0));
}

USizeCtrl::USizeCtrl(wxWindow* parent, int wxWindowID)
    : wxPanel(parent, wxWindowID)
    , _minValue(1, 1)
    , _maxValue(INT_MAX, INT_MAX)
{
    const wxSize crossSize(AABB_COMMA_WIDTH, -1);

    auto* sizer = new wxBoxSizer(wxHORIZONTAL);

    _width = new wxSpinCtrl(this, wxID_ANY);
    _width->SetRange(1, INT_MAX);
    _width->SetSizeHints(MIN_SPIN_WIDTH, -1);
    sizer->Add(_width, wxSizerFlags(1).Expand());

    auto* cross = new wxStaticText(this, wxID_ANY, "x",
                                   wxDefaultPosition, crossSize,
                                   wxALIGN_CENTER_HORIZONTAL | wxST_NO_AUTORESIZE);
    sizer->Add(cross, wxSizerFlags().Bottom().Border(wxALL, 1));

    _height = new wxSpinCtrl(this, wxID_ANY);
    _height->SetRange(1, INT_MAX);
    _height->SetSizeHints(MIN_SPIN_WIDTH, -1);
    sizer->Add(_height, wxSizerFlags(1).Expand());

    this->SetSizer(sizer);
}

void USizeCtrl::SetMaxValue(const usize& max)
{
    _maxValue = max;
    UpdateRanges();
}

void USizeCtrl::SetMinValue(const usize& min)
{
    _minValue = min;
    UpdateRanges();
}

void USizeCtrl::UpdateRanges()
{
    _width->SetRange(_minValue.width, _maxValue.width);
    _height->SetRange(_minValue.height, _maxValue.height);
}

URectCtrl::URectCtrl(wxWindow* parent, int wxWindowID)
    : wxPanel(parent, wxWindowID)
    , _range(INT_MAX, INT_MAX)
    , _minRectSize(1, 1)
    , _maxRectSize(INT_MAX, INT_MAX)
{
    const wxSize commaSize(AABB_COMMA_WIDTH, -1);

    auto* sizer = new wxBoxSizer(wxVERTICAL);

    auto* posSizer = new wxBoxSizer(wxHORIZONTAL);

    _xPos = new wxSpinCtrl(this, wxID_ANY);
    _xPos->SetSizeHints(MIN_SPIN_WIDTH, -1);
    posSizer->Add(_xPos, wxSizerFlags(1).Expand());

    auto* comma = new wxStaticText(this, wxID_ANY, ",",
                                   wxDefaultPosition, commaSize,
                                   wxALIGN_CENTER_HORIZONTAL | wxST_NO_AUTORESIZE);
    posSizer->Add(comma, wxSizerFlags(0).Bottom().Border(wxALL, 1));

    _yPos = new wxSpinCtrl(this, wxID_ANY);
    _yPos->SetSizeHints(MIN_SPIN_WIDTH, -1);
    posSizer->Add(_yPos, wxSizerFlags(1).Expand());

    sizer->Add(posSizer, wxSizerFlags(1).Expand());

    sizer->AddSpacer(wxSizerFlags::GetDefaultBorder());

    auto* sizeSizer = new wxBoxSizer(wxHORIZONTAL);

    _width = new wxSpinCtrl(this, wxID_ANY);
    _width->SetSizeHints(MIN_SPIN_WIDTH, -1);
    sizeSizer->Add(_width, wxSizerFlags(1).Expand());

    auto* cross = new wxStaticText(this, wxID_ANY, "x",
                                   wxDefaultPosition, commaSize,
                                   wxALIGN_CENTER_HORIZONTAL | wxST_NO_AUTORESIZE);
    sizeSizer->Add(cross, wxSizerFlags(0).Bottom().Border(wxALL, 1));

    _height = new wxSpinCtrl(this, wxID_ANY);
    _height->SetSizeHints(MIN_SPIN_WIDTH, -1);
    sizeSizer->Add(_height, wxSizerFlags(1).Expand());

    sizer->Add(sizeSizer, wxSizerFlags(1).Expand());

    this->SetSizer(sizer);

    UpdateRanges();

    // Events
    // ------
    _xPos->Bind(wxEVT_SPINCTRL, [this](wxSpinEvent& e) {
        _width->SetRange(_minRectSize.width, _range.width - _xPos->GetValue());
        e.Skip();
    });
    _yPos->Bind(wxEVT_SPINCTRL, [this](wxSpinEvent& e) {
        _height->SetRange(_minRectSize.height, _range.height - _yPos->GetValue());
        e.Skip();
    });
}

void URectCtrl::SetMinRectSize(const usize& min)
{
    _minRectSize = min;
    UpdateRanges();
}

void URectCtrl::SetMaxRectSize(const usize& max)
{
    _maxRectSize = max;
    UpdateRanges();
}

void URectCtrl::SetRange(const usize& range)
{
    _range = range;
    UpdateRanges();
}

void URectCtrl::UpdateRanges()
{
    _xPos->SetRange(0, _range.width - _minRectSize.width);
    _yPos->SetRange(0, _range.height - _minRectSize.height);

    _width->SetRange(_minRectSize.width,
                     std::min(_maxRectSize.width, _range.width - _xPos->GetValue()));
    _height->SetRange(_minRectSize.height,
                      std::min(_maxRectSize.height, _range.height - _yPos->GetValue()));
}
