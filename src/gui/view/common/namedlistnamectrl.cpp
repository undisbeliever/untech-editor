#include "namedlistnamectrl.h"
#include "models/common/namedlist.h"

using namespace UnTech::View;

NamedListNameCtrl::NamedListNameCtrl(wxWindow* parent, wxWindowID id,
                                     const wxString& value, const wxPoint& pos,
                                     const wxSize& size, long style)
    : wxTextCtrl(parent, id, value, pos, size, style)
{
    this->Bind(wxEVT_TEXT, [this](wxCommandEvent& e) {
        bool changed = false;
        wxString text = this->GetValue();

        for (auto c : text) {
            if (!UnTech::isNameCharValid(c)) {
                c = '_';
                changed = true;
            }
        }

        if (changed) {
            this->ChangeValue(text);
        }

        e.Skip();
    });
}
