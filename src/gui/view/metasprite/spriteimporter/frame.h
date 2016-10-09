#pragma once
#include "gui/controllers/metasprite/spriteimporter.h"
#include "gui/view/common/controllerinterface.h"
#include "gui/view/common/filedialogs.h"
#include "gui/view/metasprite/framehelper.h"
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSprite {
namespace SpriteImporter {

namespace SI = UnTech::MetaSprite::SpriteImporter;

class Frame : public wxFrame {
public:
    const static wxString WINDOW_NAME;
    const static DocumentType FRAMESET_DOCUMENT_TYPE;

public:
    Frame();

    SI::SpriteImporterController& Controller() { return _controller; }

    static void CreateOpen(const std::string& filename);

protected:
    ControllerInterface _controllerInterface;
    SI::SpriteImporterController _controller;
    FrameHelper<Frame> _frameHelper;

    wxTimer _initBugfixTimer;
};
}
}
}
}
