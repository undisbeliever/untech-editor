#pragma once
#include "gui/controllers/metasprite/spriteimporter.h"
#include <wx/notebook.h>
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSprite {
namespace SpriteImporter {

namespace SI = UnTech::MetaSprite::SpriteImporter;

class Sidebar : public wxNotebook {
public:
    Sidebar(wxWindow* parent, int wxWindowID,
            SI::SpriteImporterController& controller);

private:
    SI::SpriteImporterController& _controller;
    wxNotebook* _frameNotebook;
};
}
}
}
}
