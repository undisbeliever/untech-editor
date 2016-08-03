#pragma once
#include "gui/controllers/metasprite.h"
#include "models/common/optional.h"
#include <wx/bitmap.h>
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

class FrameGraphicsCtrl : public wxPanel {
public:
    enum class MouseState {
        NONE,
        SELECT,
        DRAG
    };
    struct MouseDrag {
        ms8rect aabb;
        point prevMouse;

        bool isActive = false;
        bool canDrag;
        bool resize;
        bool resizeLeft;
        bool resizeRight;
        bool resizeTop;
        bool resizeBottom;
    };

public:
    FrameGraphicsCtrl(wxWindow* parent, wxWindowID id,
                      MS::MetaSpriteController& controller);

    const MS::Frame* GetMetaSpriteFrame() const { return _currentFrame; }
    void SetMetaSpriteFrame(const MS::Frame* frame);

    void CenterScrollbar() { UpdateScrollbar(true); }

private:
    void OnNonBitmapDataChanged();

    void UpdateScrollbar(bool center = false);

    void UpdateBitmap();

    void Render(wxPaintDC& dc);

    optional<point> GetMousePosition();

    void OnMouseLeftDown(wxMouseEvent&);
    void OnMouseLeftUp(wxMouseEvent&);
    void OnMouseLeftDClick(wxMouseEvent&);
    void OnMouseMotion(wxMouseEvent&);

    void ResetMouseState();

    void OnMouseLeftUp_Select(const point& mouse);

    void MouseDrag_MouseClick(const point& mouse);
    void MouseDrag_ActivateDrag();
    void MouseDrag_MouseMotion(const point& mouse);
    void MouseDrag_Confirm();
    void MouseDrag_Reset();

private:
    MS::MetaSpriteController& _controller;
    const MS::Frame* _currentFrame;

    wxBitmap _bitmap;

    MouseState _mouseState;
    point _prevMouse;
    MouseDrag _mouseDrag;
};
}
}
}
