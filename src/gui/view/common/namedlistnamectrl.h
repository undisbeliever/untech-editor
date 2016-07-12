#pragma once
#include <wx/wx.h>

namespace UnTech {
namespace View {

class NamedListNameCtrl : public wxTextCtrl {
public:
    NamedListNameCtrl(wxWindow* parent, wxWindowID id,
                      const wxString& value = wxEmptyString,
                      const wxPoint& pos = wxDefaultPosition,
                      const wxSize& size = wxDefaultSize,
                      long style = 0);
};
}
}
