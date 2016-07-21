#include "view/sprite-importer/frame.h"
#include <wx/wx.h>

class MyApp : public wxApp {
public:
    virtual bool OnInit();
};

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    auto* frame = new UnTech::View::SpriteImporter::Frame();
    frame->Show(true);

    return true;
}
