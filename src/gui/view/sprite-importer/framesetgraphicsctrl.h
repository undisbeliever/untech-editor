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
    enum class MouseState {
        NONE,
        SELECT
    };
    struct MousePosition {
        upoint frameSetLoc;
        upoint frameLoc;
        bool isValid = false;
        bool isInFrame = false;
    };

public:
    FrameSetGraphicsCtrl(wxWindow* parent, wxWindowID id,
                         SI::SpriteImporterController& controller);

private:
    void OnNonBitmapDataChanged();

    void UpdateScrollbar();
    void ScrollToSelectedFrame();

    void UpdateBitmap();

    void Render(wxPaintDC& dc);

    MousePosition GetMousePosition();

    void OnMouseLeftDown(wxMouseEvent&);
    void OnMouseLeftUp(wxMouseEvent&);
    void OnMouseLeftDClick(wxMouseEvent&);

    void ResetMouseState();

    void OnMouseLeftUp_Select(const MousePosition& mouse);

private:
    SI::SpriteImporterController& _controller;

    wxBitmap _bitmap;

    MouseState _mouseState;
    MousePosition _prevMouse;
};
}
}
}
