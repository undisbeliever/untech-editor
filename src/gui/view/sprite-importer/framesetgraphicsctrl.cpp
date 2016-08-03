#include "framesetgraphicsctrl.h"
#include "gui/view/common/drawing.hpp"
#include "gui/view/common/image.h"
#include <algorithm>
#include <cassert>
#include <wx/dc.h>
#include <wx/dcgraph.h>
#include <wx/pen.h>

// ::SHOULD update image on refresh::

using namespace UnTech;
using namespace UnTech::View::SpriteImporter;

typedef SI::SpriteImporterController::SelectedTypeController::Type SelectedType;

FrameSetGraphicsCtrl::FrameSetGraphicsCtrl(wxWindow* parent, wxWindowID id,
                                           SI::SpriteImporterController& controller)
    : wxPanel(parent, id, wxDefaultPosition, wxDefaultSize,
              wxBORDER_SUNKEN | wxHSCROLL | wxVSCROLL | wxALWAYS_SHOW_SB)
    , _controller(controller)
    , _bitmap()
{
    SetAutoLayout(true);

    // Signals
    // -------
    controller.frameSetController().signal_selectedChanged().connect(sigc::mem_fun(
        *this, &FrameSetGraphicsCtrl::UpdateBitmap));
    controller.frameSetController().signal_imageChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicsCtrl::UpdateBitmap)));
    controller.frameSetController().signal_dataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicsCtrl::OnNonBitmapDataChanged)));

    controller.frameController().signal_selectedChanged().connect([this](void) {
        ScrollToSelectedFrame();
        OnNonBitmapDataChanged();
    });
    controller.frameController().signal_dataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicsCtrl::OnNonBitmapDataChanged)));
    controller.frameController().signal_listDataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicsCtrl::OnNonBitmapDataChanged)));

    controller.frameObjectController().signal_dataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicsCtrl::OnNonBitmapDataChanged)));
    controller.frameObjectController().signal_listDataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicsCtrl::OnNonBitmapDataChanged)));

    controller.actionPointController().signal_dataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicsCtrl::OnNonBitmapDataChanged)));
    controller.actionPointController().signal_listDataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicsCtrl::OnNonBitmapDataChanged)));

    controller.entityHitboxController().signal_dataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicsCtrl::OnNonBitmapDataChanged)));
    controller.entityHitboxController().signal_listDataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicsCtrl::OnNonBitmapDataChanged)));

    controller.selectedTypeController().signal_selectedChanged().connect(sigc::mem_fun(
        *this, &FrameSetGraphicsCtrl::OnNonBitmapDataChanged));

    _controller.layersController().signal_layersChanged().connect(sigc::mem_fun(
        *this, &FrameSetGraphicsCtrl::OnNonBitmapDataChanged));

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
               &FrameSetGraphicsCtrl::OnMouseLeftUp, this);

    this->Bind(wxEVT_LEFT_DOWN,
               &FrameSetGraphicsCtrl::OnMouseLeftDown, this);

    this->Bind(wxEVT_LEFT_DCLICK,
               &FrameSetGraphicsCtrl::OnMouseLeftDClick, this);

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

void FrameSetGraphicsCtrl::UpdateScrollbar()
{
    const SI::FrameSet* frameSet = _controller.frameSetController().selected();

    int clientWidth, clientHeight;
    GetClientSize(&clientWidth, &clientHeight);

    clientWidth /= _controller.settings().zoomX();
    clientHeight /= _controller.settings().zoomY();

    int width, height;
    if (_bitmap.IsOk()) {
        width = _bitmap.GetWidth();
        height = _bitmap.GetHeight();
    }
    else {
        width = clientWidth;
        height = clientHeight;
    }

    int hThumbSize = std::min(clientWidth, width);
    int vThumbSize = std::min(clientHeight, height);

    int hPos, vPos;
    if (frameSet) {
        hPos = GetScrollPos(wxHORIZONTAL) + GetScrollThumb(wxHORIZONTAL) / 2 - hThumbSize / 2;
        vPos = GetScrollPos(wxVERTICAL) + GetScrollThumb(wxVERTICAL) / 2 - vThumbSize / 2;
    }
    else {
        hPos = 0;
        vPos = 0;
    }

    this->SetScrollbar(wxHORIZONTAL, hPos, hThumbSize, width);
    this->SetScrollbar(wxVERTICAL, vPos, vThumbSize, height);

    OnNonBitmapDataChanged();
}

void FrameSetGraphicsCtrl::ScrollToSelectedFrame()
{
    auto scroll = [this](int orientation, int start, int size) {
        int thumbSize = GetScrollThumb(orientation);
        int scrollPos = GetScrollPos(orientation);

        if (start < scrollPos || (start + int(size)) > (scrollPos + thumbSize)) {
            int newPos = start + size / 2 - thumbSize / 2;
            SetScrollPos(orientation, newPos);
        }
    };

    const SI::Frame* frame = _controller.frameController().selected();
    if (frame) {
        const urect& loc = frame->location();

        scroll(wxHORIZONTAL, loc.x, loc.width);
        scroll(wxVERTICAL, loc.y, loc.height);
    }
}

void FrameSetGraphicsCtrl::OnNonBitmapDataChanged()
{
    ResetMouseState();
    Refresh(true);
}

void FrameSetGraphicsCtrl::UpdateBitmap()
{
    const SI::FrameSet* frameSet = _controller.frameSetController().selected();
    if (frameSet) {
        _bitmap = ImageToWxBitmap(frameSet->image());
    }
    else {
        _bitmap = wxNullBitmap;
    }

    UpdateScrollbar();
    OnNonBitmapDataChanged();
}

void FrameSetGraphicsCtrl::Render(wxPaintDC& paintDc)
{
    const SI::FrameSet* frameSet = _controller.frameSetController().selected();
    const auto& layers = _controller.layersController();

    if (frameSet == nullptr || paintDc.IsOk() == false) {
        return;
    }

    const double zoomX = _controller.settings().zoomX();
    const double zoomY = _controller.settings().zoomY();

    int clientWidth, clientHeight;
    paintDc.GetSize(&clientWidth, &clientHeight);

    clientWidth /= zoomX;
    clientHeight /= zoomY;

    int xPos = GetScrollPos(wxHORIZONTAL);
    int yPos = GetScrollPos(wxVERTICAL);

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

    DrawingHelper fsHelper(dc, zoomX, zoomY, xPos, yPos);

    // Origins
    // -------
    if (layers.origin()) {
        for (const auto& frameIt : frameSet->frames()) {
            const SI::Frame& frame = frameIt.second;

            DashedPen(paintDc, [&](wxDC& dc) {
                DrawingHelper helper(dc, zoomX, zoomY,
                                     xPos - frame.location().x,
                                     yPos - frame.location().y);

                helper.DrawCrossHair(frame.origin(),
                                     frame.location().size());
            });
        }
    }

    // Frame boxes
    // -----------
    {
        dc.SetBrush(wxNullBrush);
        dc.SetPen(FRAME_OUTLINE_PEN);

        for (const auto& frameIt : frameSet->frames()) {
            const SI::Frame& frame = frameIt.second;
            fsHelper.DrawRectangle(frame.location());
        }
    }

    // Frame details
    // -------------
    for (const auto& frameIt : frameSet->frames()) {
        const SI::Frame& frame = frameIt.second;

        DrawingHelper helper(dc, zoomX, zoomY,
                             xPos - frame.location().x,
                             yPos - frame.location().y);

        if (layers.frameObjects()) {
            dc.SetBrush(wxNullBrush);
            dc.SetPen(FRAME_OBJECTS_PEN);

            for (const SI::FrameObject& obj : frame.objects()) {
                helper.DrawSquare(obj.location(), obj.sizePx());
            }
        }

        if (layers.entityHitboxes()) {
            dc.SetBrush(ENTITY_HITBOX_BRUSH);
            dc.SetPen(ENTITY_HITBOX_PEN);
            for (const SI::EntityHitbox& eh : frame.entityHitboxes()) {
                helper.DrawRectangle(eh.aabb());
            }
        }

        if (frame.solid() && layers.tileHitbox()) {
            dc.SetBrush(TILE_HITBOX_BRUSH);
            dc.SetPen(TILE_HITBOX_PEN);
            helper.DrawRectangle(frame.tileHitbox());
        }

        if (layers.actionPoints()) {
            dc.SetBrush(wxNullBrush);
            dc.SetPen(ACTION_POINT_PEN);
            for (const SI::ActionPoint& ap : frame.actionPoints()) {
                helper.DrawCross(ap.location());
            }
        }
    }

    // Selected Frame
    // --------------
    const SI::Frame* selectedFrame = _controller.frameController().selected();
    if (selectedFrame) {
        fsHelper.AntiHighlightRectangle(selectedFrame->location());

        // Selected Item
        // -------------
        DrawingHelper helper(dc, zoomX, zoomY,
                             xPos - selectedFrame->location().x,
                             yPos - selectedFrame->location().y);

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

FrameSetGraphicsCtrl::MousePosition FrameSetGraphicsCtrl::GetMousePosition()
{
    const SI::FrameSet* frameSet = _controller.frameSetController().selected();

    MousePosition ret;
    ret.isValid = false;
    ret.isInFrame = false;

    if (frameSet == nullptr || frameSet->image().empty()) {
        return ret;
    }

    const wxPoint pt = ScreenToClient(wxGetMousePosition());
    const wxSize size = GetClientSize();

    if (pt.x < 0 || pt.y < 0 || pt.x >= size.GetWidth() || pt.y >= size.GetHeight()) {
        return ret;
    }

    const double zoomX = _controller.settings().zoomX();
    const double zoomY = _controller.settings().zoomY();

    const unsigned x = (pt.x / zoomX) + GetScrollPos(wxHORIZONTAL);
    const unsigned y = (pt.y / zoomY) + GetScrollPos(wxVERTICAL);

    const usize& isize = frameSet->image().size();
    if (x < isize.width && y < isize.height) {
        ret.isValid = true;
        ret.frameSetLoc = upoint(x, y);

        const SI::Frame* frame = _controller.frameController().selected();
        if (frame) {
            const urect& loc = frame->location();
            if (loc.contains(ret.frameSetLoc)) {
                ret.isInFrame = true;
                ret.frameLoc = upoint(x - loc.x, y - loc.y);
            }
        }
    }

    return ret;
}

void FrameSetGraphicsCtrl::OnMouseLeftDown(wxMouseEvent& event)
{
    MousePosition mouse = GetMousePosition();

    if (_mouseState == MouseState::NONE) {
        _mouseState = MouseState::SELECT;
        _prevMouse = mouse;
    }

    event.Skip();
}

void FrameSetGraphicsCtrl::OnMouseLeftUp(wxMouseEvent& event)
{
    MousePosition mouse = GetMousePosition();

    if (_mouseState == MouseState::SELECT) {
        OnMouseLeftUp_Select(mouse);
    }

    ResetMouseState();

    event.Skip();
}

void FrameSetGraphicsCtrl::OnMouseLeftDClick(wxMouseEvent& event)
{
    MousePosition mouse = GetMousePosition();

    // treat double clicks as select item
    OnMouseLeftUp_Select(mouse);

    ResetMouseState();

    event.Skip();
}

void FrameSetGraphicsCtrl::ResetMouseState()
{
    _mouseState = MouseState::NONE;
}

void FrameSetGraphicsCtrl::OnMouseLeftUp_Select(const MousePosition& mouse)
{
    // only select if mouse did not move.
    if (mouse.frameSetLoc != _prevMouse.frameSetLoc) {
        return;
    }

    if (!mouse.isValid) {
        _controller.frameController().setSelected(nullptr);
        return;
    }
    else if (mouse.isInFrame) {
        /*
         * Select a given item inside frame.
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

        const SI::Frame* frame = _controller.frameController().selected();
        const auto& layers = _controller.layersController();
        const auto& fMouse = mouse.frameLoc;

        assert(frame != nullptr);

        if (layers.frameObjects()) {
            for (const SI::FrameObject& obj : frame->objects()) {
                if (urect(obj.location(), obj.sizePx()).contains(fMouse)) {
                    updateMatch(SelectedType::FRAME_OBJECT, &obj);
                }
            }
        }

        if (layers.actionPoints()) {
            for (const SI::ActionPoint& ap : frame->actionPoints()) {
                if (fMouse == ap.location()) {
                    updateMatch(SelectedType::ACTION_POINT, &ap);
                }
            }
        }

        if (layers.entityHitboxes()) {
            for (const SI::EntityHitbox& eh : frame->entityHitboxes()) {
                if (eh.aabb().contains(fMouse)) {
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
    else {
        // select frame
        const SI::FrameSet* frameSet = _controller.frameSetController().selected();
        assert(frameSet != nullptr);

        for (const auto fIt : frameSet->frames()) {
            const auto frameLoc = fIt.second.location();

            if (frameLoc.contains(mouse.frameSetLoc)) {
                _controller.frameController().setSelected(&fIt.second);
                return;
            }
        }

        _controller.frameController().setSelected(nullptr);
    }
}
