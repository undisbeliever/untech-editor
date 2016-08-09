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

    this->Bind(wxEVT_MOTION,
               &FrameGraphicsCtrl::OnMouseMotion, this);

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
        dc.SetBackground(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE)));
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

        case SelectedType::TILE_HITBOX:
            if (frame->solid()) {
                helper.DrawSelectedRectangle(frame->tileHitbox());
            }
            break;
        }
    }

    // Drag shadow
    // -----------
    if (_mouseState == MouseState::DRAG && _mouseDrag.isActive) {
        dc.SetPen(DRAG_PEN);
        dc.SetBrush(DRAG_BRUSH);
        helper.DrawRectangle(_mouseDrag.aabb);
    }
}

optional<point> FrameGraphicsCtrl::GetMousePosition()
{
    if (_currentFrame == nullptr) {
        return optional<point>();
    }

    const wxPoint pt = ScreenToClient(wxGetMousePosition());
    const wxSize size = GetClientSize();

    if (pt.x < 0 || pt.y < 0 || pt.x >= size.GetWidth() || pt.y >= size.GetHeight()) {
        return optional<point>();
    }

    const double zoomX = _controller.settings().zoomX();
    const double zoomY = _controller.settings().zoomY();

    point ret(
        (pt.x / zoomX) + GetScrollPos(wxHORIZONTAL) - FRAME_IMAGE_OFFSET,
        (pt.y / zoomY) + GetScrollPos(wxVERTICAL) - FRAME_IMAGE_OFFSET);

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

    if (isValid(ret.x) && isValid(ret.y)) {
        return ret;
    }
    else {
        return optional<point>();
    }
}

void FrameGraphicsCtrl::OnMouseLeftDown(wxMouseEvent& event)
{
    auto mouse = GetMousePosition();

    if (mouse) {
        if (_mouseState == MouseState::NONE) {
            MouseDrag_MouseClick(mouse.value());

            _mouseState = MouseState::SELECT;
            _prevMouse = mouse.value();
        }
    }

    event.Skip();
}

void FrameGraphicsCtrl::OnMouseLeftUp(wxMouseEvent& event)
{
    auto mouse = GetMousePosition();

    if (mouse) {
        switch (_mouseState) {
        case MouseState::NONE:
            break;

        case MouseState::SELECT:
            OnMouseLeftUp_Select(mouse.value());
            break;

        case MouseState::DRAG:
            MouseDrag_MouseMotion(mouse.value());
            MouseDrag_Confirm();
            break;
        }
    }

    ResetMouseState();

    event.Skip();
}

void FrameGraphicsCtrl::OnMouseLeftDClick(wxMouseEvent& event)
{
    auto mouse = GetMousePosition();

    if (mouse) {
        OnMouseLeftUp_Select(mouse.value());
    }

    ResetMouseState();

    event.Skip();
}

void FrameGraphicsCtrl::OnMouseMotion(wxMouseEvent& event)
{
    auto mouse = GetMousePosition();

    if (mouse) {
        if (_mouseState == MouseState::SELECT) {
            if (_mouseDrag.canDrag) {
                _mouseState = MouseState::DRAG;
            }
        }

        if (_mouseState == MouseState::DRAG) {
            MouseDrag_MouseMotion(mouse.value());
        }
    }

    event.Skip();
}

void FrameGraphicsCtrl::ResetMouseState()
{
    MouseDrag_Reset();
    _mouseState = MouseState::NONE;
}

void FrameGraphicsCtrl::OnMouseLeftUp_Select(const point& mouse)
{
    assert(_currentFrame);

    // only select if mouse did not move.
    if (mouse != _prevMouse) {
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
            if (ms8rect(obj.location(), obj.sizePx()).contains(mouse)) {
                updateMatch(SelectedType::FRAME_OBJECT, &obj);
            }
        }
    }

    if (layers.actionPoints()) {
        for (const MS::ActionPoint& ap : _currentFrame->actionPoints()) {
            if (mouse == ap.location()) {
                updateMatch(SelectedType::ACTION_POINT, &ap);
            }
        }
    }

    if (layers.entityHitboxes()) {
        for (const MS::EntityHitbox& eh : _currentFrame->entityHitboxes()) {
            if (eh.aabb().contains(mouse)) {
                updateMatch(SelectedType::ENTITY_HITBOX, &eh);
            }
        }
    }

    if (layers.tileHitbox()) {
        if (_currentFrame->solid()) {
            if (_currentFrame->tileHitbox().contains(mouse)) {
                updateMatch(SelectedType::TILE_HITBOX, _currentFrame);
            }
        }
    }

    if (match.type == SelectedType::NONE) {
        // handle wrap around.
        match = firstMatch;
    }

    _controller.selectedTypeController().selectItem(match.type, match.item);
}

// Mouse Drag
// ==========

void FrameGraphicsCtrl::MouseDrag_MouseClick(const point& mouse)
{
    auto& md = _mouseDrag;
    bool canResize = false;

    switch (_controller.selectedTypeController().type()) {
    case SelectedType::NONE:
        md.isActive = false;
        md.canDrag = false;
        return;

    case SelectedType::FRAME_OBJECT:
        if (const auto* fo = _controller.frameObjectController().selected()) {
            md.aabb = ms8rect(fo->location(), fo->sizePx());
        }
        break;

    case SelectedType::ACTION_POINT:
        if (const auto* ap = _controller.actionPointController().selected()) {
            md.aabb = ms8rect(ap->location(), 1);
        }
        break;

    case SelectedType::ENTITY_HITBOX:
        if (const auto* eh = _controller.entityHitboxController().selected()) {
            md.aabb = eh->aabb();
            canResize = true;
        }
        break;

    case SelectedType::TILE_HITBOX:
        if (const auto* frame = _controller.frameController().selected()) {
            if (frame->solid()) {
                md.aabb = frame->tileHitbox();
                canResize = true;
            }
        }
        break;
    }

    if (canResize) {
        md.resizeLeft = mouse.x <= md.aabb.left() + 1;
        md.resizeRight = mouse.x == md.aabb.right() - 1 || mouse.x == md.aabb.right();

        md.resizeTop = mouse.y <= md.aabb.top() + 1;
        md.resizeBottom = mouse.y == md.aabb.bottom() - 1 || mouse.y == md.aabb.bottom();

        md.resize = md.resizeLeft | md.resizeRight | md.resizeTop | md.resizeBottom;
        md.canDrag = md.resize | md.aabb.contains(mouse);
    }
    else {
        md.resize = false;
        md.canDrag = md.aabb.contains(mouse);
    }

    md.prevMouse = mouse;
}

void FrameGraphicsCtrl::MouseDrag_MouseMotion(const point& mouse)
{
    auto& md = _mouseDrag;

    if (md.canDrag == false) {
        return;
    }

    if (md.isActive == false) {
        MouseDrag_ActivateDrag();
    }

    if (md.prevMouse != mouse) {
        ms8rect newAabb = md.aabb;

        if (!md.resize) {
            // move
            newAabb.x += mouse.x - md.prevMouse.x;
            newAabb.y += mouse.y - md.prevMouse.y;
        }
        else {
            // resize
            int rx = mouse.x;
            if (md.resizeLeft) {
                if (rx >= newAabb.right()) {
                    rx = newAabb.right() - 1;
                }
                newAabb.width = newAabb.right() - rx;
                newAabb.x = rx;
            }
            else if (md.resizeRight) {
                if (rx <= newAabb.left()) {
                    rx = newAabb.left() + 1;
                }
                newAabb.width = rx - newAabb.x;
            }

            int ry = mouse.y;
            if (md.resizeTop) {
                if (ry >= newAabb.bottom()) {
                    ry = newAabb.bottom() - 1;
                }
                newAabb.height = newAabb.bottom() - ry;
                newAabb.y = ry;
            }
            else if (md.resizeBottom) {
                if (ry <= newAabb.top()) {
                    ry = newAabb.top() + 1;
                }
                newAabb.height = ry - newAabb.y;
            }
        }

        if (md.aabb != newAabb) {
            md.aabb = newAabb;
            Refresh(true);
        }

        md.prevMouse = mouse;
    }
}

void FrameGraphicsCtrl::MouseDrag_ActivateDrag()
{
    auto& md = _mouseDrag;

    if (md.isActive == false && md.canDrag) {
        if (!md.resize) {
            SetCursor(wxCursor(wxCURSOR_SIZING));
        }
        else {
            bool horizontal = md.resizeLeft | md.resizeRight;
            bool vertical = md.resizeTop | md.resizeBottom;

            if (horizontal && !vertical) {
                SetCursor(wxCursor(wxCURSOR_SIZEWE));
            }
            else if (vertical && !horizontal) {
                SetCursor(wxCursor(wxCURSOR_SIZENS));
            }
            else if ((md.resizeLeft && md.resizeTop)
                     || (md.resizeRight && md.resizeBottom)) {
                SetCursor(wxCursor(wxCURSOR_SIZENWSE));
            }
            else if ((md.resizeRight && md.resizeTop)
                     || (md.resizeLeft && md.resizeBottom)) {
                SetCursor(wxCursor(wxCURSOR_SIZENESW));
            }
        }

        CaptureMouse();
    }

    md.isActive = md.canDrag;
}

void FrameGraphicsCtrl::MouseDrag_Confirm()
{
    auto& md = _mouseDrag;

    if (md.isActive) {
        switch (_controller.selectedTypeController().type()) {
        case SelectedType::NONE:
            break;

        case SelectedType::FRAME_OBJECT:
            _controller.frameObjectController().selected_setLocation(ms8point(md.aabb.x, md.aabb.y));
            break;

        case SelectedType::ACTION_POINT:
            _controller.actionPointController().selected_setLocation(ms8point(md.aabb.x, md.aabb.y));
            break;

        case SelectedType::ENTITY_HITBOX:
            _controller.entityHitboxController().selected_setAabb(md.aabb);
            break;

        case SelectedType::TILE_HITBOX:
            _controller.frameController().selected_setTileHitbox(md.aabb);
            break;
        }

        MouseDrag_Reset();
    }
}

void FrameGraphicsCtrl::MouseDrag_Reset()
{
    auto& md = _mouseDrag;

    if (md.isActive) {
        md.isActive = false;
        md.canDrag = false;

        SetCursor(wxNullCursor);
        ReleaseMouse();
        Refresh(true);
    }
}
