/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once
#include "gui-wx/controllers/metasprite/metasprite.h"
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
