#pragma once
#include "gui/controllers/metasprite/metasprite.h"
#include <wx/bitmap.h>
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSprite {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite::MetaSprite;

class TilesetCtrl : public wxPanel {
public:
    const static unsigned SMALL_SIZE = 8;
    const static unsigned LARGE_SIZE = 16;

    enum class MouseState {
        NONE,
        SELECT,
        DRAW
    };
    struct MousePosition {
        bool isValid = false;
        bool isSmall;
        unsigned tileId;
        unsigned tileX;
        unsigned tileY;
    };

public:
    TilesetCtrl(wxWindow* parent, wxWindowID id,
                MS::MetaSpriteController& controller);

private:
    void UpdateSize();
    void UpdateScrollbar();
    void ScrollTo(int x, unsigned width);

    void CreateBitmaps();
    void CreateSmallBitmap();
    void CreateLargeBitmap();

    void Render(wxDC& dc);

    // Will only return a valid position if clicked on a tile
    MousePosition GetMousePosition();

    void OnMouseLeftDown(wxMouseEvent&);
    void OnMouseLeftUp(wxMouseEvent&);
    void OnMouseMotion(wxMouseEvent&);
    void ResetMouseState();

    void DrawTilePixel(const MousePosition&);
    void SelectColorWithMouse(const MousePosition&);

private:
    MS::MetaSpriteController& _controller;

    wxBitmap _smallTilesBitmap;
    wxBitmap _largeTilesBitmap;

    MouseState _mouseState;
    MousePosition _prevMouse;
};
}
}
}
}
