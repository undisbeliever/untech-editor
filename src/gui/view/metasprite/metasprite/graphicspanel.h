/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once
#include "gui/controllers/metasprite/metasprite.h"
#include <array>
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSprite {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite::MetaSprite;

class FrameGraphicsCtrl;
class TilesetCtrl;

class GraphicsPanel : public wxPanel {
    const static unsigned N_FRAME_CTRLS = 2;

public:
    GraphicsPanel(wxWindow* parent, wxWindowID id,
                  MS::MetaSpriteController& controller);

    void SetSplit(bool enabled);
    void CenterMetaSpriteFrames();

private:
    MS::MetaSpriteController& _controller;

    std::array<FrameGraphicsCtrl*, N_FRAME_CTRLS> _frames;
    TilesetCtrl* _tileset;

    unsigned _currentFrameCtrl;
    bool _split;
};
}
}
}
}
