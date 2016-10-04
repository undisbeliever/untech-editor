#pragma once
#include "gui/controllers/metasprite-common.h"
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSpriteCommon {

namespace MSC = UnTech::MetaSpriteCommon;

class AnimationSidebarPage : public wxPanel {
public:
    AnimationSidebarPage(wxWindow* parent, int wxWindowID,
                         MSC::AbstractFrameSetController& controller);
};
}
}
}
