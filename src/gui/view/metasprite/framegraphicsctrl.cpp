#include "framegraphicsctrl.h"
#include "gui/view/common/drawing.hpp"
#include "gui/view/snes/tile.hpp"
#include <algorithm>
#include <cassert>
#include <wx/dc.h>
#include <wx/dcgraph.h>
#include <wx/pen.h>

using namespace UnTech;
using namespace UnTech::View::MetaSprite;

const static int BITMAP_SIZE = -UnTech::int_ms8_t::MIN * 2;
const static int FRAME_IMAGE_OFFSET = -UnTech::int_ms8_t::MIN;

typedef MS::MetaSpriteController::SelectedTypeController::Type SelectedType;

FrameGraphicsCtrl::FrameGraphicsCtrl(wxWindow* parent, wxWindowID id,
                                     MS::MetaSpriteController& controller)
    : wxPanel(parent, id, wxDefaultPosition, wxDefaultSize,
              wxBORDER_SUNKEN | wxHSCROLL | wxVSCROLL | wxALWAYS_SHOW_SB)
    , _controller(controller)
    , _bitmap(BITMAP_SIZE, BITMAP_SIZE, 24)
{
    SetAutoLayout(true);
    UpdateBitmap();

    // Signals
    // -------
    controller.frameController().signal_dataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameGraphicsCtrl::OnNonBitmapDataChanged)));

    controller.frameController().signal_listChanged().connect([this](void) {
        SetMetaSpriteFrame(nullptr);
    });
    controller.frameController().signal_listDataChanged().connect([this](const MS::Frame::list_t*) {
        auto list = _controller.frameController().list();
        if (!list || !list->contains(_currentFrame)) {
            SetMetaSpriteFrame(nullptr);
        }
        OnNonBitmapDataChanged();
    });

    controller.paletteController().signal_selectedChanged().connect(sigc::mem_fun(
        *this, &FrameGraphicsCtrl::UpdateBitmap));
    controller.paletteController().signal_selectedDataChanged().connect(sigc::mem_fun(
        *this, &FrameGraphicsCtrl::UpdateBitmap));

    controller.frameSetController().signal_smallTilesetChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameGraphicsCtrl::UpdateBitmap)));
    controller.frameSetController().signal_largeTilesetChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameGraphicsCtrl::UpdateBitmap)));

    controller.frameObjectController().signal_dataChanged().connect(
        [this](const MS::FrameObject* obj) {
            if (obj && &obj->frame() == _controller.frameController().selected()) {
                UpdateBitmap();
            }
        });
    controller.frameObjectController().signal_listDataChanged().connect(
        [this](const MS::FrameObject::list_t* list) {
            const MS::Frame* frame = _controller.frameController().selected();
            if (frame && list == &frame->objects()) {
                UpdateBitmap();
            }
        });

    controller.actionPointController().signal_dataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameGraphicsCtrl::OnNonBitmapDataChanged)));
    controller.actionPointController().signal_listDataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameGraphicsCtrl::OnNonBitmapDataChanged)));

    controller.entityHitboxController().signal_dataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameGraphicsCtrl::OnNonBitmapDataChanged)));
    controller.entityHitboxController().signal_listDataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameGraphicsCtrl::OnNonBitmapDataChanged)));

    controller.selectedTypeController().signal_selectedChanged().connect(sigc::mem_fun(
        *this, &FrameGraphicsCtrl::OnNonBitmapDataChanged));

    _controller.layersController().signal_layersChanged().connect(sigc::mem_fun(
        *this, &FrameGraphicsCtrl::OnNonBitmapDataChanged));

    _controller.settings().signal_zoomChanged().connect([this](void) {
        UpdateScrollbar();
        OnNonBitmapDataChanged();
    });

    // Events
    // ------
    this->Bind(wxEVT_PAINT, [this](wxPaintEvent&) {
        wxPaintDC dc(this);
        Render(dc);
    });

    this->Bind(wxEVT_SIZE, [this](wxSizeEvent& e) {
        UpdateScrollbar();
        OnNonBitmapDataChanged();
        e.Skip();
    });

    this->Bind(wxEVT_LEFT_UP,
               &FrameGraphicsCtrl::OnMouseLeftUp, this);

    this->Bind(wxEVT_LEFT_DOWN,
               &FrameGraphicsCtrl::OnMouseLeftDown, this);

    this->Bind(wxEVT_LEFT_DCLICK,
               &FrameGraphicsCtrl::OnMouseLeftDClick, this);

    this->Bind(wxEVT_LEAVE_WINDOW, [this](wxMouseEvent& e) {
        ResetMouseState();
        e.Skip();
    });

    auto scrollEvent = [this](wxScrollWinEvent& e) {
        OnNonBitmapDataChanged();
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

void FrameGraphicsCtrl::SetMetaSpriteFrame(const MS::Frame* frame)
{
    if (_currentFrame != frame) {
        _currentFrame = frame;

        if (frame == nullptr) {
            CenterScrollbar();
        }
        UpdateBitmap();
    }
}

void FrameGraphicsCtrl::UpdateScrollbar(bool center)
{
    int clientWidth, clientHeight;
    GetClientSize(&clientWidth, &clientHeight);

    clientWidth /= _controller.settings().zoomX();
    clientHeight /= _controller.settings().zoomY();

    int hThumbSize = std::min(clientWidth, BITMAP_SIZE);
    int vThumbSize = std::min(clientHeight, BITMAP_SIZE);

    int hPos, vPos;
    if (_currentFrame && !center) {
        hPos = GetScrollPos(wxHORIZONTAL) + GetScrollThumb(wxHORIZONTAL) / 2 - hThumbSize / 2;
        vPos = GetScrollPos(wxVERTICAL) + GetScrollThumb(wxVERTICAL) / 2 - vThumbSize / 2;
    }
    else {
        hPos = FRAME_IMAGE_OFFSET - hThumbSize / 2;
        vPos = FRAME_IMAGE_OFFSET - vThumbSize / 2;
    }

    this->SetScrollbar(wxHORIZONTAL, hPos,
                       hThumbSize, BITMAP_SIZE);

    this->SetScrollbar(wxVERTICAL, vPos,
                       vThumbSize, BITMAP_SIZE);

    OnNonBitmapDataChanged();
}

void FrameGraphicsCtrl::OnNonBitmapDataChanged()
{
    ResetMouseState();
    Refresh(true);
}

void FrameGraphicsCtrl::UpdateBitmap()
{
    typedef MS::FrameObject::ObjectSize OS;

    OnNonBitmapDataChanged();

    // Clear the bitmap
    {
        wxMemoryDC dc;
        dc.SelectObject(_bitmap);
        dc.SetBackground(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND)));
        dc.Clear();
    }

    // Draw the sprites
    const MS::Frame* frame = _currentFrame;
    const MS::Palette* palette = _controller.paletteController().selected();
    if (frame && palette) {
        const MS::FrameSet& frameSet = frame->frameSet();

        wxNativePixelData pData(_bitmap);

        for (unsigned order = 0; order < 4; order++) {
            for (auto it = frame->objects().rbegin(); it != frame->objects().rend(); ++it) {
                const MS::FrameObject& obj = *it;

                if (obj.order() == order) {
                    const unsigned tileId = obj.tileId();

                    if (obj.size() == OS::SMALL) {
                        const auto& smallTileset = frameSet.smallTileset();

                        if (tileId < smallTileset.size()) {
                            UnTech::View::Snes::DrawTileTransparent(
                                pData,
                                smallTileset.tile(tileId), *palette,
                                obj.location().x + FRAME_IMAGE_OFFSET,
                                obj.location().y + FRAME_IMAGE_OFFSET,
                                obj.hFlip(), obj.vFlip());
                        }
                    }
                    else {
                        const auto& largeTileset = frameSet.largeTileset();

                        if (tileId < largeTileset.size()) {
                            UnTech::View::Snes::DrawTileTransparent(
                                pData,
                                largeTileset.tile(tileId), *palette,
                                obj.location().x + FRAME_IMAGE_OFFSET,
                                obj.location().y + FRAME_IMAGE_OFFSET,
                                obj.hFlip(), obj.vFlip());
                        }
                    }
                }
            }
        }
    }
}

void FrameGraphicsCtrl::Render(wxPaintDC& paintDc)
{
    const MS::Frame* frame = _currentFrame;
    const auto& layers = _controller.layersController();

    if (frame == nullptr || paintDc.IsOk() == false) {
        return;
    }

    const double zoomX = _controller.settings().zoomX();
    const double zoomY = _controller.settings().zoomY();

    int clientWidth, clientHeight;
    GetClientSize(&clientWidth, &clientHeight);

    clientWidth /= zoomX;
    clientHeight /= zoomY;

    int xPos = GetScrollPos(wxHORIZONTAL);
    if (clientWidth > BITMAP_SIZE) {
        xPos -= (clientWidth - BITMAP_SIZE) / 2;
    }

    int yPos = GetScrollPos(wxVERTICAL);
    if (clientHeight > BITMAP_SIZE) {
        yPos -= (clientHeight - BITMAP_SIZE) / 2;
    }

    // Draw Bitmap
    // -----------
    if (_bitmap.IsOk()) {
        wxMemoryDC tmpDc;
        tmpDc.SelectObjectAsSource(_bitmap);

        int width = _bitmap.GetWidth() - xPos;
        int height = _bitmap.GetHeight() - yPos;

        if (width > 0 && height > 0) {
            paintDc.StretchBlit(0, 0, width * zoomX, height * zoomY,
                                &tmpDc,
                                xPos, yPos, width, height);
        }
    }

    // This DC is used for transparency stuff
    wxGCDC dc(paintDc);
    if (dc.IsOk() == false) {
        return;
    }
    wxGraphicsContext* gc = dc.GetGraphicsContext();
    gc->SetAntialiasMode(wxANTIALIAS_NONE);
    gc->SetInterpolationQuality(wxINTERPOLATION_NONE);

    DrawingHelper helper(dc, zoomX, zoomY,
                         xPos - FRAME_IMAGE_OFFSET,
                         yPos - FRAME_IMAGE_OFFSET);

    // Origin
    // ------
    if (layers.origin()) {
        dc.SetPen(ORIGIN_PEN);
        dc.CrossHair((FRAME_IMAGE_OFFSET - xPos) * zoomX,
                     (FRAME_IMAGE_OFFSET - yPos) * zoomY);
    }

    // Boxes
    // -----
    if (layers.frameObjects()) {
        dc.SetBrush(wxNullBrush);
        dc.SetPen(FRAME_OBJECTS_PEN);

        for (const MS::FrameObject& obj : frame->objects()) {
            helper.DrawSquare(obj.location(), obj.sizePx());
        }
    }

    if (layers.entityHitboxes()) {
        dc.SetBrush(ENTITY_HITBOX_BRUSH);
        dc.SetPen(ENTITY_HITBOX_PEN);
        for (const MS::EntityHitbox& eh : frame->entityHitboxes()) {
            helper.DrawRectangle(eh.aabb());
        }
    }

    if (frame->solid() && layers.tileHitbox()) {
        dc.SetBrush(TILE_HITBOX_BRUSH);
        dc.SetPen(TILE_HITBOX_PEN);
        helper.DrawRectangle(frame->tileHitbox());
    }

    if (layers.actionPoints()) {
        dc.SetBrush(wxNullBrush);
        dc.SetPen(ACTION_POINT_PEN);
        for (const MS::ActionPoint& ap : frame->actionPoints()) {
            helper.DrawCross(ap.location());
        }
    }

    // Selected Item
    // -------------
    if (frame == _controller.frameController().selected()) {
        switch (_controller.selectedTypeController().type()) {
        case SelectedType::NONE:
            break;

        case SelectedType::FRAME_OBJECT:
            if (const auto* fo = _controller.frameObjectController().selected()) {
                helper.DrawSelectedSquare(fo->location(), fo->sizePx());
            }
            break;

        case SelectedType::ENTITY_HITBOX:
            if (const auto* eh = _controller.entityHitboxController().selected()) {
                helper.DrawSelectedRectangle(eh->aabb());
            }
            break;

        case SelectedType::ACTION_POINT:
            if (const auto* ap = _controller.actionPointController().selected()) {
                helper.DrawSelectedCross(ap->location());
            }
            break;
        }
    }
}

FrameGraphicsCtrl::MousePosition FrameGraphicsCtrl::GetMousePosition()
{
    MousePosition ret;
    ret.isValid = false;

    if (_currentFrame == nullptr) {
        return ret;
    }

    const wxPoint pt = ScreenToClient(wxGetMousePosition());
    const wxSize size = GetClientSize();

    if (pt.x < 0 || pt.y < 0 || pt.x >= size.GetWidth() || pt.y >= size.GetHeight()) {
        return ret;
    }

    const double zoomX = _controller.settings().zoomX();
    const double zoomY = _controller.settings().zoomY();

    ret.x = (pt.x / zoomX) + GetScrollPos(wxHORIZONTAL) - FRAME_IMAGE_OFFSET;
    ret.y = (pt.y / zoomY) + GetScrollPos(wxVERTICAL) - FRAME_IMAGE_OFFSET;

    int clientWidth, clientHeight;
    GetClientSize(&clientWidth, &clientHeight);
    clientWidth /= zoomX;
    clientHeight /= zoomY;

    if (clientWidth > BITMAP_SIZE) {
        ret.x -= (clientWidth - BITMAP_SIZE) / 2;
    }
    if (clientHeight > BITMAP_SIZE) {
        ret.y -= (clientHeight - BITMAP_SIZE) / 2;
    }

    auto isValid = [](int v) {
        return v >= -FRAME_IMAGE_OFFSET && v < (BITMAP_SIZE - FRAME_IMAGE_OFFSET);
    };
    ret.isValid = isValid(ret.x) && isValid(ret.y);

    return ret;
}

void FrameGraphicsCtrl::OnMouseLeftDown(wxMouseEvent& event)
{
    MousePosition mouse = GetMousePosition();

    if (mouse.isValid) {
        if (_mouseState == MouseState::NONE) {
            _mouseState = MouseState::SELECT;
            _prevMouse = mouse;
        }
    }

    event.Skip();
}

void FrameGraphicsCtrl::OnMouseLeftUp(wxMouseEvent& event)
{
    MousePosition mouse = GetMousePosition();

    if (mouse.isValid) {
        if (_mouseState == MouseState::SELECT) {
            OnMouseLeftUp_Select(mouse);
        }
    }

    ResetMouseState();

    event.Skip();
}

void FrameGraphicsCtrl::OnMouseLeftDClick(wxMouseEvent& event)
{
    MousePosition mouse = GetMousePosition();

    if (mouse.isValid) {
        OnMouseLeftUp_Select(mouse);
    }

    ResetMouseState();

    event.Skip();
}

void FrameGraphicsCtrl::ResetMouseState()
{
    _mouseState = MouseState::NONE;
}

void FrameGraphicsCtrl::OnMouseLeftUp_Select(const MousePosition& mouse)
{
    assert(mouse.isValid);
    assert(_currentFrame);

    // only select if mouse did not move.
    if (mouse.x != _prevMouse.x || mouse.y != _prevMouse.y) {
        return;
    }

    const auto& layers = _controller.layersController();

    /*
     * Select a given item.
     *
     * This code cycles through the given selections.
     * On click, the next item is selected. If the last item
     * was the previously selected one then the first match
     * is selected.
     */
    const void* current = _controller.selectedTypeController().selectedPtr();

    struct SelHandler {
        SelectedType type = SelectedType::NONE;
        const void* item = nullptr;
    };
    SelHandler match, firstMatch;

    auto updateMatch = [&](SelectedType type, const void* item) mutable {
        if (match.type == SelectedType::NONE) {
            match.type = type;
            match.item = item;

            if (firstMatch.type == SelectedType::NONE) {
                firstMatch = match;
            }
        }
        if (item == current) {
            match.type = SelectedType::NONE;
        }
    };

    if (layers.frameObjects()) {
        for (const MS::FrameObject& obj : _currentFrame->objects()) {
            const auto loc = obj.location();

            if (mouse.x >= loc.x && mouse.x < (loc.x + (int)obj.sizePx())
                && mouse.y >= loc.y && mouse.y < (loc.y + (int)obj.sizePx())) {

                updateMatch(SelectedType::FRAME_OBJECT, &obj);
            }
        }
    }

    if (layers.actionPoints()) {
        for (const MS::ActionPoint& ap : _currentFrame->actionPoints()) {
            const auto loc = ap.location();

            if (mouse.x == loc.x && mouse.y == loc.y) {

                updateMatch(SelectedType::ACTION_POINT, &ap);
            }
        }
    }

    if (layers.entityHitboxes()) {
        for (const MS::EntityHitbox& eh : _currentFrame->entityHitboxes()) {
            const auto& aabb = eh.aabb();

            if (mouse.x >= aabb.left() && mouse.x < aabb.right()
                && mouse.y >= aabb.top() && mouse.y < aabb.bottom()) {

                updateMatch(SelectedType::ENTITY_HITBOX, &eh);
            }
        }
    }

    if (match.type == SelectedType::NONE) {
        // handle wrap around.
        match = firstMatch;
    }

    _controller.selectedTypeController().selectItem(match.type, match.item);
}
