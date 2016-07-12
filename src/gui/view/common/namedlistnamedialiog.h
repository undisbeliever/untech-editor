#pragma once
#include "namedlistnamectrl.h"

namespace UnTech {
namespace View {

class NamedListNameDialog : public wxDialog {
public:
    NamedListNameDialog(wxWindow* parent,
                        const wxString& title, const wxString& caption);

    wxString GetValue() const { return _name->GetValue(); }
    void SetValue(const wxString& text);

private:
    NamedListNameCtrl* _name;
};
}
}
