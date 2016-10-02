#pragma once

#include "idstringtextctrl.h"
#include <functional>

namespace UnTech {
namespace View {

class IdStringDialog : public wxDialog {
public:
    IdStringDialog(wxWindow* parent,
                   const wxString& title, const wxString& caption,
                   std::function<bool(const std::string&)> validator);

    void SetIdString(const idstring& text);

    idstring GetIdString() const
    {
        return _idText->GetIdString();
    }

private:
    void OnTextChanged();

private:
    std::function<bool(const idstring&)> _validator;
    IdStringTextCtrl* _idText;
};
}
}
