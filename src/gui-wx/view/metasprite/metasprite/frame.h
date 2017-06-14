/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once
#include "gui-wx/controllers/metasprite/metasprite.h"
#include "gui-wx/view/common/controllerinterface.h"
#include "gui-wx/view/common/dontmergefocushack.h"
#include "gui-wx/view/common/filedialogs.h"
#include "gui-wx/view/metasprite/animation/previewpanel.h"
#include "gui-wx/view/metasprite/framehelper.h"
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
    DontMergeFocusHack _dontMergeFocusHack;
    FrameHelper<Frame> _frameHelper;

    wxTimer _initBugfixTimer;

    GraphicsPanel* _graphics;
    Animation::PreviewPanel* _animationPreview;
    Sidebar* _sidebar;
};
}
}
}
}
