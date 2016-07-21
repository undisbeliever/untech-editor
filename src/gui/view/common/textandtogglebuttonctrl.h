#pragma once

#include <wx/tglbtn.h>
#include <wx/wx.h>

namespace UnTech {
namespace View {

class TextAndToggleButtonCtrl : public wxPanel {
public:
    TextAndToggleButtonCtrl(wxWindow* parent, int wxWindowID);

    wxString GetTextValue() { return _text->GetValue(); }

    // Will also right align text
    void ChangeTextValue(const wxString& str);

    bool GetButtonValue() { return _button->GetValue(); }
    void SetButtonValue(bool v) { return _button->SetValue(v); }

    wxTextCtrl* GetText() { return _text; };
    wxToggleButton* GetButton() { return _button; };

private:
    wxTextCtrl* _text;
    wxToggleButton* _button;
};
}
}
