#include "namedlistnamedialiog.h"
#include "gui/view/defaults.h"

using namespace UnTech::View;

NamedListNameDialog::NamedListNameDialog(wxWindow* parent,
                                         const wxString& title, const wxString& caption,
                                         std::function<bool(const std::string&)> validator)
    : wxDialog(parent, wxID_ANY, title,
               wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxCAPTION | wxCENTRE)
    , _validator(validator)
{
    // layout and borders match that of wxWidgets' wxTextEntryDialog

    auto* sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(sizer);

    sizer->Add(CreateTextSizer(caption),
               wxSizerFlags().DoubleBorder());

    _name = new NamedListNameCtrl(this, wxID_ANY);
    _name->SetSizeHints(DIALOG_TEXT_WIDTH, -1);
    sizer->Add(_name, wxSizerFlags().Expand().TripleBorder(wxLEFT | wxRIGHT));

    sizer->Add(CreateSeparatedButtonSizer(wxOK | wxCANCEL),
               wxSizerFlags().Expand().DoubleBorder());

    SetAutoLayout(true);
    sizer->SetSizeHints(this);
    sizer->Fit(this);

    OnTextChanged();

    this->Bind(wxEVT_SHOW, [this](wxShowEvent&) {
        _name->SelectAll();
        _name->SetFocus();
    });

    this->Bind(wxEVT_TEXT, [this](wxCommandEvent&) {
        OnTextChanged();
    });
}

void NamedListNameDialog::SetValue(const wxString& text)
{
    _name->ChangeValue(text);
    OnTextChanged();
}

void NamedListNameDialog::OnTextChanged()
{
    bool v = _validator(_name->GetValue().ToStdString());

    // not sure how to do this normally
    wxWindow* ok = wxWindow::FindWindowById(wxID_OK, this);
    if (ok) {
        ok->Enable(v);
    }
}
