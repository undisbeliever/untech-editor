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
        SELECT
    };
    struct MousePosition {
        // can't use ms8point because x/y can be > int_ms8_t::MAX
        int x, y;
        bool isValid = false;
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

    MousePosition GetMousePosition();

    void OnMouseLeftDown(wxMouseEvent&);
    void OnMouseLeftUp(wxMouseEvent&);
    void OnMouseLeftDClick(wxMouseEvent&);

    void ResetMouseState();

    void OnMouseLeftUp_Select(const MousePosition& mouse);

private:
    MS::MetaSpriteController& _controller;
    const MS::Frame* _currentFrame;

    wxBitmap _bitmap;

    MouseState _mouseState;
    MousePosition _prevMouse;
};
}
}
}
