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
