#pragma once
#include "gui/controllers/metasprite/animation.h"
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
