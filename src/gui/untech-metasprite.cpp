#include "view/metasprite/window.h"
#include <wx/wx.h>

class MyApp : public wxApp {
public:
    virtual bool OnInit();
};

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    auto* window = new UnTech::View::MetaSprite::Window();
    window->Controller().newDocument();
    window->Show(true);

    return true;
}
