#pragma once
#include "gui/controllers/metasprite.h"
#include <wx/bitmap.h>
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

class FrameGraphicsCtrl : public wxPanel {
public:
    FrameGraphicsCtrl(wxWindow* parent, wxWindowID id,
                      MS::MetaSpriteController& controller);

private:
    void Refresh() { wxPanel::Refresh(true); }

    void UpdateScrollbar();

    void UpdateBitmap();

    void Render(wxPaintDC& dc);

private:
    MS::MetaSpriteController& _controller;

    wxBitmap _bitmap;
};
}
}
}
