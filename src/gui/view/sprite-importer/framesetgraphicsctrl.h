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
        SELECT,
        DRAG
    };
    struct MousePosition {
        upoint frameSetLoc;
        upoint frameLoc;
        bool isValid = false;
        bool isInFrame = false;
    };
    struct MouseDrag {
        urect aabb;
        const SI::Frame* frame;
        upoint prevMouse;

        bool isActive = false;
        bool canDrag;
        bool resize;
        bool resizeLeft;
        bool resizeRight;
        bool resizeTop;
        bool resizeBottom;
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
    void OnMouseMotion(wxMouseEvent&);

    void ResetMouseState();

    void OnMouseLeftUp_Select(const MousePosition& mouse);

    void MouseDrag_MouseClick(const MousePosition& mouse);
    void MouseDrag_ActivateDrag();
    void MouseDrag_MouseMotion(const MousePosition& mouse);
    void MouseDrag_Confirm();
    void MouseDrag_Reset();

private:
    SI::SpriteImporterController& _controller;

    wxBitmap _bitmap;

    MouseState _mouseState;
    MousePosition _prevMouse;
    MouseDrag _mouseDrag;
};
}
}
}
