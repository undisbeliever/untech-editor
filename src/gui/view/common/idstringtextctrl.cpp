/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

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
