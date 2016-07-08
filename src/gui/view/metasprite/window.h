#pragma once
#include "gui/controllers/metasprite.h"
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

class Window : public wxFrame {
public:
    Window();

private:
    MS::MetaSpriteController _controller;
};
}
}
}
