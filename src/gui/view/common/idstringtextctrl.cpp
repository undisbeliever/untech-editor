#include "idstringtextctrl.h"

using namespace UnTech::View;

IdStringTextCtrl::IdStringTextCtrl(wxWindow* parent, wxWindowID id,
                                   const idstring& value, const wxPoint& pos,
                                   const wxSize& size, long style)
    : wxTextCtrl(parent, id, wxString(value), pos, size, style)
{
    this->Bind(wxEVT_TEXT, [this](wxCommandEvent& e) {
        bool changed = false;
        wxString text = this->GetValue();

        for (auto c : text) {
            if (!idstring::isCharValid(c)) {
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
