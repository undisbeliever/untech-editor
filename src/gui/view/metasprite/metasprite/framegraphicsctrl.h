/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once
#include "gui/controllers/metasprite/metasprite.h"
#include "models/common/optional.h"
#include <wx/bitmap.h>
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSprite {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite::MetaSprite;

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

    const idstring& GetCurrentFrameId() const { return _currentFrameId; }
    void SetCurrentFrameId(const idstring& frameId);

    void CenterScrollbar() { UpdateScrollbar(true); }

private:
    const MS::Frame& CurrentFrame();

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
    idstring _currentFrameId;

    wxBitmap _bitmap;

    MouseState _mouseState;
    point _prevMouse;
    MouseDrag _mouseDrag;
};
}
}
}
}
