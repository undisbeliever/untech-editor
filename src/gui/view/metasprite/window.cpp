#include "window.h"
#include "sidebar.h"
#include "gui/view/defaults.h"

namespace UnTech {
namespace View {
namespace MetaSprite {

enum MENU_IDS {
    ID_CREATE = 1000,
    ID_CLONE,
    ID_REMOVE,
    ID_MOVE_UP,
    ID_MOVE_DOWN,
    ID_ZOOM_1,
    ID_ZOOM_2,
    ID_ZOOM_3,
    ID_ZOOM_4,
    ID_ZOOM_5,
    ID_ZOOM_6,
    ID_ZOOM_7,
    ID_ZOOM_8,
    ID_ZOOM_9,
    ID_ASPECT_SQUARE,
    ID_ASPECT_NTSC,
    ID_ASPECT_PAL,
};
}
}
}

using namespace UnTech::View::MetaSprite;

Window::Window()
    : wxFrame(NULL, wxID_ANY, "UnTech", wxDefaultPosition, wxSize(280, 180))
{
    // Widgets
    // =======
    {
        auto* sizer = new wxBoxSizer(wxHORIZONTAL);

        // ::TODO replace::
        auto* graphics = new wxPanel(this, wxID_ANY);
        graphics->SetBackgroundColour(wxColour("#4f5049"));

        auto* sidebar = new Sidebar(this, wxID_ANY);

        sizer->Add(graphics, 4, wxEXPAND | wxALL, DEFAULT_BORDER);
        sizer->Add(sidebar, 0, wxEXPAND | (wxALL ^ wxLEFT), DEFAULT_BORDER);

        this->SetSizer(sizer);

        Centre();
    }

    // Menus
    // =====
    {
        auto* menuBar = new wxMenuBar();

        auto* file = new wxMenu();
        file->Append(wxID_NEW, "&New\tCTRL+N");
        file->Append(wxID_OPEN, "&Open\tCTRL+O");
        file->Append(wxID_SAVE, "&Save\tCTRL+S");
        file->Append(wxID_SAVEAS, "Save &As\tCTRL+SHIFT+S");
        file->AppendSeparator();
        file->Append(wxID_EXIT, "&Exit\tALT+F4");

        auto* edit = new wxMenu();
        edit->Append(wxID_UNDO);
        edit->Append(wxID_REDO);
        edit->AppendSeparator();
        edit->Append(ID_CREATE, "Create");
        edit->Append(ID_CLONE, "Clone Selected\tCTRL+D");
        edit->Append(ID_REMOVE, "Remove Selected");
        edit->Append(ID_MOVE_UP, "Move Selected Up");
        edit->Append(ID_MOVE_DOWN, "Move Selected Down");

        auto* zoom = new wxMenu();
        zoom->Append(wxID_ZOOM_IN, "Zoom &In\tCTRL++");
        zoom->Append(wxID_ZOOM_OUT, "Zoom &Out\tCTRL+-");
        zoom->AppendRadioItem(ID_ZOOM_1, "1\tALT+1");
        zoom->AppendRadioItem(ID_ZOOM_2, "2\tALT+2");
        zoom->AppendRadioItem(ID_ZOOM_3, "3\tALT+3");
        zoom->AppendRadioItem(ID_ZOOM_4, "4\tALT+4");
        zoom->AppendRadioItem(ID_ZOOM_5, "5\tALT+5");
        zoom->AppendRadioItem(ID_ZOOM_6, "6\tALT+6");
        zoom->AppendRadioItem(ID_ZOOM_7, "7\tALT+7");
        zoom->AppendRadioItem(ID_ZOOM_8, "8\tALT+8");
        zoom->AppendRadioItem(ID_ZOOM_9, "9\tALT+9");

        auto* aspectRatio = new wxMenu();
        aspectRatio->AppendRadioItem(ID_ASPECT_SQUARE, "&Square");
        aspectRatio->AppendRadioItem(ID_ASPECT_NTSC, "&NTSC");
        aspectRatio->AppendRadioItem(ID_ASPECT_PAL, "&PAL");

        auto* view = new wxMenu();
        view->AppendSubMenu(zoom, "&Zoom");
        view->AppendSubMenu(aspectRatio, "&Aspect Ratio");

        menuBar->Append(file, "&File");
        menuBar->Append(edit, "&Edit");
        menuBar->Append(view, "&View");

        this->SetMenuBar(menuBar);

        // EVENTS
        // ------
        menuBar->Bind(
            wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent&) {
                this->Close(true);
            },
            wxID_EXIT);

        edit->Bind(
            wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent&) {
                this->Close(true);
            },
            ID_REMOVE);

        zoom->Bind(
            wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent& e) {
                int z = e.GetId() - ID_ZOOM_1 + 1;
                printf("Set Zoom: %i\n", z);
            },
            ID_ZOOM_1, ID_ZOOM_9);

        aspectRatio->Bind(
            wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent& e) {
                int a = e.GetId() - ID_ASPECT_SQUARE;
                printf("Set Aspect Ratio: %i\n", a);
            },
            ID_ASPECT_SQUARE, ID_ASPECT_PAL);
    }
}
