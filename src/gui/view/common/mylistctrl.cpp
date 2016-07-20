#include "mylistctrl.h"
#include "gui/view/defaults.h"

using namespace UnTech::View;

void MyListCtrl::ResizeColumnsEqually()
{
    int count = GetColumnCount();
    int width = GetClientSize().GetWidth() / count;

    if (width < MIN_AUTO_COLUMN_WIDTH) {
        width = MIN_AUTO_COLUMN_WIDTH;
    }

    for (int i = 0; i < count; i++) {
        SetColumnWidth(i, width);
    }
}

void MyListCtrl::BindColumnsToEqualWidth()
{
    this->Bind(wxEVT_SIZE, [this](wxSizeEvent& e) {
        ResizeColumnsEqually();

        e.Skip();
    });
}

void MyListCtrl::HideHeader()
{
    SetWindowStyle(GetWindowStyle() | wxLC_NO_HEADER);
}
