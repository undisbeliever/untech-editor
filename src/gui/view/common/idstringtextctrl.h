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
