#include "tilesetctrl.h"
#include "gui/view/common/drawing.hpp"
#include "gui/view/snes/tile.hpp"
#include <algorithm>
#include <cassert>
#include <wx/dc.h>
#include <wx/pen.h>

using namespace UnTech;
using namespace UnTech::View::MetaSprite;

// ::TODO add UpdateSmallBitmap(tileId), UpdateLargeBitmap(tileId) functions::

TilesetCtrl::TilesetCtrl(wxWindow* parent, wxWindowID id,
                         MS::MetaSpriteController& controller)
    : wxPanel(parent, id, wxDefaultPosition, wxDefaultSize,
              wxBORDER_SUNKEN | wxHSCROLL | wxALWAYS_SHOW_SB)
    , _controller(controller)
    , _smallTilesBitmap(wxNullBitmap)
    , _largeTilesBitmap(wxNullBitmap)
    , _mouseState(MouseState::NONE)
    , _prevMouse()
{
    SetAutoLayout(true);
    UpdateSize();
    CreateBitmaps();

    // Signals
    // -------

    controller.paletteController().signal_selectedChanged().connect([this](void) {
        ResetMouseState();
        CreateBitmaps();
    });

    controller.frameSetController().signal_selectedChanged().connect([this](void) {
        UpdateSize();
        ResetMouseState();
        CreateBitmaps();
    });

    controller.paletteController().signal_selectedDataChanged().connect([this](void) {
        ResetMouseState();
        CreateBitmaps();
    });

    _controller.paletteController().signal_selectedColorChanged().connect([this](void) {
        if (_controller.paletteController().selectedColorId() >= 0) {
            SetCursor(wxCursor(wxCURSOR_PENCIL));
        }
        else {
            SetCursor(wxNullCursor);
        }
        ResetMouseState();
    });

    controller.frameSetController().signal_tileCountChanged().connect([this](const MS::FrameSet* fs) {
        if (fs && fs == _controller.frameSetController().selected()) {
            ResetMouseState();
            CreateBitmaps();
        }
    });

    controller.frameSetController().signal_smallTilesetChanged().connect([this](const MS::FrameSet* fs) {
        if (fs && fs == _controller.frameSetController().selected()) {
            CreateSmallBitmap();
        }
    });

    controller.frameSetController().signal_largeTilesetChanged().connect([this](const MS::FrameSet* fs) {
        if (fs && fs == _controller.frameSetController().selected()) {
            CreateLargeBitmap();
        }
    });

    controller.frameObjectController().signal_selectedChanged().connect([this](void) {
        Refresh(true);
    });

    controller.frameObjectController().signal_selectedDataChanged().connect([this](void) {
        Refresh(true);
    });

    _controller.settings().signal_zoomChanged().connect([this](void) {
        ResetMouseState();
        UpdateSize();
        UpdateScrollbar();
        Refresh(true);
    });

    // Events
    // ------
    this->Bind(wxEVT_PAINT, [this](wxPaintEvent&) {
        wxPaintDC dc(this);
        Render(dc);
    });

    this->Bind(wxEVT_SIZE, [this](wxSizeEvent& e) {
        UpdateScrollbar();
        Refresh();
        e.Skip();
    });

    this->Bind(wxEVT_LEFT_UP,
               &TilesetCtrl::OnMouseLeftUp, this);

    this->Bind(wxEVT_LEFT_DOWN,
               &TilesetCtrl::OnMouseLeftDown, this);

    this->Bind(wxEVT_MOTION,
               &TilesetCtrl::OnMouseMotion, this);

    this->Bind(wxEVT_LEAVE_WINDOW, [this](wxMouseEvent& e) {
        ResetMouseState();
        e.Skip();
    });

    auto scrollEvent = [this](wxScrollWinEvent& e) {
        Refresh();
        e.Skip();
    };
    this->Bind(wxEVT_SCROLLWIN_TOP, scrollEvent);
    this->Bind(wxEVT_SCROLLWIN_BOTTOM, scrollEvent);
    this->Bind(wxEVT_SCROLLWIN_LINEUP, scrollEvent);
    this->Bind(wxEVT_SCROLLWIN_LINEDOWN, scrollEvent);
    this->Bind(wxEVT_SCROLLWIN_PAGEUP, scrollEvent);
    this->Bind(wxEVT_SCROLLWIN_PAGEDOWN, scrollEvent);
    this->Bind(wxEVT_SCROLLWIN_THUMBTRACK, scrollEvent);
    this->Bind(wxEVT_SCROLLWIN_THUMBRELEASE, scrollEvent);
}

void TilesetCtrl::UpdateSize()
{
    int h = (SMALL_SIZE + LARGE_SIZE) * _controller.settings().zoomY() + 1;
    wxSize s(-1, h);

    this->SetMinClientSize(s);
    this->SetMaxClientSize(s);

    // ::BUGFIX force layout on parent::
    wxWindow* p = this->GetParent();
    if (p) {
        p->PostSizeEvent();
    }
}

void TilesetCtrl::UpdateScrollbar()
{
    const MS::FrameSet* frameSet = _controller.frameSetController().selected();

    int width = 1;

    if (frameSet) {
        width = std::max(frameSet->smallTileset().size() * SMALL_SIZE,
                         frameSet->largeTileset().size() * LARGE_SIZE);
    }

    int clientWidth, clientHeight;
    GetClientSize(&clientWidth, &clientHeight);

    clientWidth /= _controller.settings().zoomX();

    int thumbSize = std::min(clientWidth, width);

    this->SetScrollbar(wxHORIZONTAL,
                       GetScrollPos(wxHORIZONTAL),
                       thumbSize, width);
}

void TilesetCtrl::CreateBitmaps()
{
    UpdateScrollbar();
    CreateSmallBitmap();
    CreateLargeBitmap();
}

void TilesetCtrl::CreateSmallBitmap()
{
    const MS::Palette* palette = _controller.paletteController().selected();
    const MS::FrameSet* frameSet = _controller.frameSetController().selected();

    Refresh(true);

    if (frameSet == nullptr || palette == nullptr) {
        _smallTilesBitmap = wxNullBitmap;
        return;
    }

    const auto& smallTileset = frameSet->smallTileset();
    if (smallTileset.size() == 0) {
        _smallTilesBitmap = wxNullBitmap;
        return;
    }

    const int width = smallTileset.size() * SMALL_SIZE;
    const int height = SMALL_SIZE;

    if (!_smallTilesBitmap.IsOk()
        || _smallTilesBitmap.GetWidth() != width
        || _smallTilesBitmap.GetHeight() != height) {

        _smallTilesBitmap.Create(width, height, 24);
    }

    wxNativePixelData pData(_smallTilesBitmap);

    for (unsigned i = 0; i < smallTileset.size(); i++) {
        UnTech::View::Snes::DrawTileOpaque(
            pData, smallTileset.tile(i), *palette,
            i * SMALL_SIZE, 0, false, false);
    }
}

void TilesetCtrl::CreateLargeBitmap()
{
    const MS::Palette* palette = _controller.paletteController().selected();
    const MS::FrameSet* frameSet = _controller.frameSetController().selected();

    Refresh(true);

    if (frameSet == nullptr || palette == nullptr) {
        _largeTilesBitmap = wxNullBitmap;
        return;
    }

    const auto& largeTileset = frameSet->largeTileset();
    if (largeTileset.size() == 0) {
        _largeTilesBitmap = wxNullBitmap;
        return;
    }

    const int width = largeTileset.size() * LARGE_SIZE;
    const int height = LARGE_SIZE;

    if (!_largeTilesBitmap.IsOk()
        || _largeTilesBitmap.GetWidth() != width
        || _largeTilesBitmap.GetHeight() != height) {

        _largeTilesBitmap.Create(width, height, 24);
    }

    wxNativePixelData pData(_largeTilesBitmap);

    for (unsigned i = 0; i < largeTileset.size(); i++) {
        UnTech::View::Snes::DrawTileOpaque(
            pData, largeTileset.tile(i), *palette,
            i * LARGE_SIZE, 0, false, false);
    }
}

void TilesetCtrl::Render(wxDC& dc)
{
    // I have no idea why I can't put a space between the two tilesets.
    // It works at zoom levels 1-2, but not 3-9

    const MS::FrameSet* frameSet = _controller.frameSetController().selected();

    if (frameSet == nullptr) {
        return;
    }

    const double zoomX = _controller.settings().zoomX();
    const double zoomY = _controller.settings().zoomY();

    const int xOffset = GetScrollPos(wxHORIZONTAL);
    int largeTilesOffset = SMALL_SIZE * zoomY;

    auto drawScaledBitmap = [=](wxDC& dc, const wxBitmap& bitmap, int yOffset) {
        if (bitmap.IsOk()) {
            wxMemoryDC tmpDc;
            tmpDc.SelectObjectAsSource(bitmap);

            int width = bitmap.GetWidth() - xOffset;
            int height = bitmap.GetHeight();

            dc.StretchBlit(0, yOffset, width * zoomX, height * zoomY,
                           &tmpDc,
                           xOffset, 0, width, height);
        }
    };

    drawScaledBitmap(dc, _smallTilesBitmap, 0);
    drawScaledBitmap(dc, _largeTilesBitmap, largeTilesOffset);

    // Grid
    // ----
    int width, height;
    dc.GetSize(&width, &height);

    DC::DashedPen(dc, [=](wxDC& dc) {
        dc.DrawLine(0, largeTilesOffset, width, largeTilesOffset);

        auto draw = [=](wxDC& dc, unsigned nTiles, int size, int yOffset) {
            int start = (size - (xOffset % size));
            int end = std::min((int)(nTiles * size - xOffset),
                               (int)(width / zoomX));

            for (int xPos = start; xPos <= end; xPos += size) {
                dc.DrawLine(xPos * zoomX, yOffset,
                            xPos * zoomX, yOffset + size * zoomY);
            }
        };
        draw(dc, frameSet->smallTileset().size(), SMALL_SIZE, 0);
        draw(dc, frameSet->largeTileset().size(), LARGE_SIZE, largeTilesOffset);
    });

    // Selected Tile
    // -------------
    const MS::FrameObject* obj = _controller.frameObjectController().selected();
    if (obj) {
        typedef UnTech::MetaSprite::FrameObject::ObjectSize OS;
        const int x = obj->tileId() * obj->sizePx();
        const int y = obj->size() == OS::SMALL ? 0 : SMALL_SIZE;

        DC::DrawSelectedRectangleOffset(dc, zoomX, zoomY, xOffset, 0,
                                        x, y, obj->sizePx(), obj->sizePx());
    }
}

TilesetCtrl::MousePosition TilesetCtrl::GetMousePosition()
{
    MousePosition ret;
    ret.isValid = false;

    const MS::FrameSet* frameSet = _controller.frameSetController().selected();
    if (frameSet == nullptr) {
        return ret;
    }

    const wxPoint pt = ScreenToClient(wxGetMousePosition());
    const wxSize size = GetClientSize();

    if (pt.x < 0 || pt.y < 0 || pt.x >= size.GetWidth() || pt.y >= size.GetHeight()) {
        return ret;
    }

    const double zoomX = _controller.settings().zoomX();
    const double zoomY = _controller.settings().zoomY();

    const unsigned px = (pt.x / zoomX) + GetScrollPos(wxHORIZONTAL);
    const unsigned py = pt.y / zoomY;

    if (py < SMALL_SIZE) {
        // small tile
        unsigned tileId = px / SMALL_SIZE;

        if (tileId < frameSet->smallTileset().size()) {
            ret.isValid = true;
            ret.isSmall = true;
            ret.tileId = px / SMALL_SIZE;
            ret.tileX = px % SMALL_SIZE;
            ret.tileY = py;
        }
    }
    else if (py < (SMALL_SIZE + LARGE_SIZE)) {
        // large tile
        unsigned tileId = px / LARGE_SIZE;

        if (tileId < frameSet->largeTileset().size()) {
            ret.isValid = true;
            ret.isSmall = false;
            ret.tileId = px / LARGE_SIZE;
            ret.tileX = px % LARGE_SIZE;
            ret.tileY = py - SMALL_SIZE;
        }
    }

    return ret;
}

void TilesetCtrl::OnMouseLeftDown(wxMouseEvent& event)
{
    MousePosition mouse = GetMousePosition();

    if (mouse.isValid) {
        if (_controller.paletteController().selectedColorId() >= 0) {
            _mouseState = MouseState::DRAW;

            DrawTilePixel(mouse);
            CaptureMouse();
        }
        else {
            _mouseState = MouseState::SELECT;
        }
        _prevMouse = mouse;
    }
    else {
        _mouseState = MouseState::SELECT;
    }

    event.Skip();
}

void TilesetCtrl::OnMouseLeftUp(wxMouseEvent& event)
{
    typedef MS::FrameObject::ObjectSize OS;

    MousePosition mouse = GetMousePosition();

    if (mouse.isValid) {
        if (_mouseState == MouseState::SELECT) {
            if (mouse.isSmall == _prevMouse.isSmall && mouse.tileId == _prevMouse.tileId) {
                OS size = mouse.isSmall ? OS::SMALL : OS::LARGE;
                _controller.frameObjectController().selected_setSizeAndTileId(size, mouse.tileId);
            }
        }
    }

    ResetMouseState();

    event.Skip();
}

void TilesetCtrl::OnMouseMotion(wxMouseEvent& event)
{
    if (_mouseState == MouseState::DRAW) {
        MousePosition mouse = GetMousePosition();

        if (mouse.isValid) {
            if (mouse.isSmall == _prevMouse.isSmall) {
                if (mouse.tileId != _prevMouse.tileId
                    || mouse.tileX != _prevMouse.tileX
                    || mouse.tileY != _prevMouse.tileY) {

                    DrawTilePixel(mouse);
                    _prevMouse = mouse;
                }
            }
        }
    }

    event.Skip();
}

void TilesetCtrl::ResetMouseState()
{
    if (_mouseState == MouseState::DRAW) {
        _controller.dontMergeNextAction();
        ReleaseMouse();
    }

    _mouseState = MouseState::NONE;
}

void TilesetCtrl::DrawTilePixel(const MousePosition& mouse)
{
    assert(mouse.isValid);

    if (mouse.isSmall) {
        // small
        _controller.frameSetController().selected_smallTileset_setPixel(
            mouse.tileId,
            mouse.tileX, mouse.tileY,
            _controller.paletteController().selectedColorId());
    }
    else {
        // large
        _controller.frameSetController().selected_largeTileset_setPixel(
            mouse.tileId,
            mouse.tileX, mouse.tileY,
            _controller.paletteController().selectedColorId());
    }
}
