/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once
#include "gui-wx/controllers/metasprite/animation.h"
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSprite {
namespace Animation {

namespace MSA = UnTech::MetaSprite::Animation;

class AnimationSidebarPage : public wxPanel {

public:
    AnimationSidebarPage(wxWindow* parent, int wxWindowID,
                         MSA::AnimationControllerInterface& controller);
};
}
}
}
}