#pragma once
#include "namedlistnamectrl.h"
#include <functional>

namespace UnTech {
namespace View {

class NamedListNameDialog : public wxDialog {
public:
    NamedListNameDialog(wxWindow* parent,
                        const wxString& title, const wxString& caption,
                        std::function<bool(const std::string&)> validator);

    wxString GetValue() const
    {
        return _name->GetValue();
    }
    void SetValue(const wxString& text);

private:
    void OnTextChanged();

private:
    std::function<bool(const std::string&)> _validator;
    NamedListNameCtrl* _name;
};
}
}
