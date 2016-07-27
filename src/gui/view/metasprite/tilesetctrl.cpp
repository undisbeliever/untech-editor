#include "tilesetctrl.h"
#include "gui/view/common/image.h"
#include "models/snes/tile.hpp"
#include <algorithm>
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
{
    SetAutoLayout(true);
    UpdateSize();
    CreateBitmaps();

    // Signals
    // -------

    controller.paletteController().signal_selectedChanged().connect(sigc::mem_fun(
        *this, &TilesetCtrl::CreateBitmaps));

    controller.paletteController().signal_selectedDataChanged().connect(sigc::mem_fun(
        *this, &TilesetCtrl::CreateBitmaps));

    controller.frameSetController().signal_tileCountChanged().connect([this](const MS::FrameSet* fs) {
        if (fs && fs == _controller.frameSetController().selected()) {
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

    const unsigned width = smallTileset.size() * SMALL_SIZE;
    const unsigned height = SMALL_SIZE;

    UnTech::Image buffer(width, height);

    for (unsigned i = 0; i < smallTileset.size(); i++) {
        smallTileset.tile(i).drawOpaque(buffer, *palette,
                                        i * SMALL_SIZE, 0,
                                        false, false);
    }

    _smallTilesBitmap = ImageToWxBitmap(buffer);
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

    const unsigned width = largeTileset.size() * LARGE_SIZE;
    const unsigned height = LARGE_SIZE;

    UnTech::Image buffer(width, height);

    for (unsigned i = 0; i < largeTileset.size(); i++) {
        largeTileset.tile(i).drawOpaque(buffer, *palette,
                                        i * LARGE_SIZE, 0,
                                        false, false);
    }

    _largeTilesBitmap = ImageToWxBitmap(buffer);
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

    auto drawGrid = [=](wxDC& dc) {
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
    };

    dc.SetPen(wxPen(*wxBLACK, 1, wxPENSTYLE_SOLID));
    drawGrid(dc);

    dc.SetPen(wxPen(*wxWHITE, 1, wxPENSTYLE_SHORT_DASH));
    drawGrid(dc);

    // Selected Tile
    // -------------
    const MS::FrameObject* obj = _controller.frameObjectController().selected();
    if (obj) {
        typedef UnTech::MetaSprite::FrameObject::ObjectSize OS;

        const int xT = obj->tileId() * obj->sizePx() - xOffset;

        const int x = xT * zoomX;
        const int xEnd = (xT + obj->sizePx()) * zoomX;

        const int y = obj->size() == OS::SMALL ? 0 : largeTilesOffset;

        const int width = xEnd - x;
        const int height = obj->sizePx() * zoomY;

        dc.SetBrush(wxNullBrush);

        dc.SetPen(wxPen(*wxWHITE, 1, wxPENSTYLE_SOLID));
        dc.DrawRectangle(x + 1, y + 1, width - 1, height - 1);

        dc.SetPen(wxPen(*wxBLACK, 1, wxPENSTYLE_SOLID));
        dc.DrawRectangle(x, y, width + 1, height + 1);
    }
}
