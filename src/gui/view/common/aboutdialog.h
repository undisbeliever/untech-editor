/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <wx/wx.h>

namespace UnTech {
namespace View {

class AboutDialog : public wxDialog {
public:
    AboutDialog(wxWindow* parent, const wxString& appName);

private:
    wxStaticText* CreateHText(const wxString& label);
    wxStaticText* CreateLargeHText(const wxString& label);
};
}
}
