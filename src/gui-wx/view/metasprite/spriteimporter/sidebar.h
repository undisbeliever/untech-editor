/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once
#include "gui-wx/controllers/metasprite/spriteimporter.h"
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
