#pragma once
#include "gui/controllers/sprite-importer.h"
#include <wx/bitmap.h>
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

class FrameSetGraphicsCtrl : public wxPanel {

public:
    FrameSetGraphicsCtrl(wxWindow* parent, wxWindowID id,
                         SI::SpriteImporterController& controller);

private:
    void Refresh() { wxPanel::Refresh(true); }

    void UpdateScrollbar();
    void ScrollToSelectedFrame();

    void UpdateBitmap();

    void Render(wxPaintDC& dc);

private:
    SI::SpriteImporterController& _controller;

    wxBitmap _bitmap;
};
}
}
}
