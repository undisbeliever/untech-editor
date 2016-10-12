#pragma once
#include "gui/controllers/metasprite/metasprite.h"
#include <wx/notebook.h>
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSprite {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite::MetaSprite;

class Sidebar : public wxNotebook {
public:
    Sidebar(wxWindow* parent, int wxWindowID,
            MS::MetaSpriteController& controller);

private:
    MS::MetaSpriteController& _controller;
    wxNotebook* _frameNotebook;
};
}
}
}
}