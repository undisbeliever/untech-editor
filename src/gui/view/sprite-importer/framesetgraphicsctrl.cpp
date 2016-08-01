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
        *this, &FrameSetGraphicsCtrl::Refresh)));

    controller.frameController().signal_selectedChanged().connect([this](void) {
        ScrollToSelectedFrame();
        Refresh();
    });
    controller.frameController().signal_dataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicsCtrl::Refresh)));
    controller.frameController().signal_listDataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicsCtrl::Refresh)));

    controller.frameObjectController().signal_dataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicsCtrl::Refresh)));
    controller.frameObjectController().signal_listDataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicsCtrl::Refresh)));

    controller.actionPointController().signal_dataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicsCtrl::Refresh)));
    controller.actionPointController().signal_listDataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicsCtrl::Refresh)));

    controller.entityHitboxController().signal_dataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicsCtrl::Refresh)));
    controller.entityHitboxController().signal_listDataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicsCtrl::Refresh)));

    controller.selectedTypeController().signal_selectedChanged().connect(sigc::mem_fun(
        *this, &FrameSetGraphicsCtrl::Refresh));

    _controller.layersController().signal_layersChanged().connect(sigc::mem_fun(
        *this, &FrameSetGraphicsCtrl::Refresh));

    _controller.settings().signal_zoomChanged().connect([this](void) {
        UpdateScrollbar();
        Refresh();
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

    Refresh();
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
    Refresh();
}

void FrameSetGraphicsCtrl::Render(wxPaintDC& paintDc)
{
    const SI::FrameSet* frameSet = _controller.frameSetController().selected();
    ;
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
