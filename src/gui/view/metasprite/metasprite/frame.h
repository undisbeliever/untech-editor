#pragma once
#include "gui/controllers/metasprite/metasprite.h"
#include "gui/view/common/controllerinterface.h"
#include "gui/view/common/filedialogs.h"
#include "gui/view/metasprite/framehelper.h"
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSprite {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite::MetaSprite;

class GraphicsPanel;
class Sidebar;

class Frame : public wxFrame {
public:
    const static wxString WINDOW_NAME;
    const static DocumentType FRAMESET_DOCUMENT_TYPE;

public:
    Frame();

    MS::MetaSpriteController& Controller() { return _controller; }

    static void CreateOpen(const std::string& filename);

private:
    ControllerInterface _controllerInterface;
    MS::MetaSpriteController _controller;
    FrameHelper<Frame> _frameHelper;

    wxTimer _initBugfixTimer;

    GraphicsPanel* _graphics;
    Sidebar* _sidebar;
};
}
}
}
}