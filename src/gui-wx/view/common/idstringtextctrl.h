/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/idstring.h"
#include <wx/wx.h>

namespace UnTech {
namespace View {

class IdStringTextCtrl : public wxTextCtrl {
public:
    IdStringTextCtrl(wxWindow* parent, wxWindowID id,
                     const idstring& value = idstring(),
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     long style = 0);

    inline idstring GetIdString() const
    {
        return GetValue().ToStdString();
    }
};
}
}
