#include "textandtogglebuttonctrl.h"

using namespace UnTech::View;

TextAndToggleButtonCtrl::TextAndToggleButtonCtrl(wxWindow* parent, int wxWindowID)
    : wxPanel(parent, wxWindowID)
{
    auto* box = new wxBoxSizer(wxHORIZONTAL);
    this->SetSizer(box);

    _text = new wxTextCtrl(this, wxID_ANY);
    _text->Disable();
    box->Add(_text, wxSizerFlags(1).Expand());

    _button = new wxToggleButton(this, wxID_ANY, " ... ",
                                 wxDefaultPosition, wxDefaultSize,
                                 wxBU_EXACTFIT);
    box->Add(_button, wxSizerFlags().Expand().Border(wxLEFT));
}

void TextAndToggleButtonCtrl::ChangeTextValue(const wxString& str)
{
    _text->ChangeValue(str);
    _text->SetInsertionPoint(_text->GetLastPosition());
}
