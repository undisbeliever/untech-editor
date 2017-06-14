/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

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
