#pragma once
#include <wx/listctrl.h>

namespace UnTech {
namespace View {

class MyListCtrl : public wxListCtrl {
public:
    using wxListCtrl::wxListCtrl;

    void ResizeColumnsEqually();
    void BindColumnsToEqualWidth();
    void HideHeader();
};
}
}
