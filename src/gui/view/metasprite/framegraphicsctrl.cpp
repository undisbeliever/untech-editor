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
    controller.frameController().signal_selectedChanged().connect(sigc::mem_fun(
        *this, &FrameGraphicsCtrl::UpdateBitmap));
    controller.frameController().signal_selectedDataChanged().connect(sigc::mem_fun(
        *this, &FrameGraphicsCtrl::Refresh));

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
        *this, &FrameGraphicsCtrl::Refresh)));
    controller.actionPointController().signal_listDataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameGraphicsCtrl::Refresh)));

    controller.entityHitboxController().signal_dataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameGraphicsCtrl::Refresh)));
    controller.entityHitboxController().signal_listDataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameGraphicsCtrl::Refresh)));

    controller.selectedTypeController().signal_selectedChanged().connect(sigc::mem_fun(
        *this, &FrameGraphicsCtrl::Refresh));

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

void FrameGraphicsCtrl::UpdateScrollbar()
{
    const MS::Frame* frame = _controller.frameController().selected();

    int clientWidth, clientHeight;
    GetClientSize(&clientWidth, &clientHeight);

    clientWidth /= _controller.settings().zoomX();
    clientHeight /= _controller.settings().zoomY();

    int hThumbSize = std::min(clientWidth, BITMAP_SIZE);
    int vThumbSize = std::min(clientHeight, BITMAP_SIZE);

    int hPos, vPos;
    if (frame) {
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
}

void FrameGraphicsCtrl::UpdateBitmap()
{
    typedef MS::FrameObject::ObjectSize OS;

    Refresh();

    // Clear the bitmap
    {
        wxMemoryDC dc;
        dc.SelectObject(_bitmap);
        dc.SetBackground(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND)));
        dc.Clear();
    }

    // Draw the sprites
    const MS::Frame* frame = _controller.frameController().selected();
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
    const MS::Frame* frame = _controller.frameController().selected();

    if (frame == nullptr) {
        return;
    }

    const double zoomX = _controller.settings().zoomX();
    const double zoomY = _controller.settings().zoomY();

    int clientWidth, clientHeight;
    paintDc.GetSize(&clientWidth, &clientHeight);

    clientWidth /= zoomX;
    clientHeight /= zoomY;

    int xOffset = GetScrollPos(wxHORIZONTAL);
    if (clientWidth > BITMAP_SIZE) {
        xOffset -= (clientWidth - BITMAP_SIZE) / 2;
    }

    int yOffset = GetScrollPos(wxVERTICAL);
    if (clientHeight > BITMAP_SIZE) {
        yOffset -= (clientHeight - BITMAP_SIZE) / 2;
    }

    // Draw Bitmap
    // -----------
    {
        wxMemoryDC tmpDc;
        tmpDc.SelectObjectAsSource(_bitmap);

        int width = _bitmap.GetWidth() - xOffset;
        int height = _bitmap.GetHeight() - yOffset;

        paintDc.StretchBlit(0, 0, width * zoomX, height * zoomY,
                            &tmpDc,
                            xOffset, yOffset, width, height);
    }

    // This DC is used for transparency stuff
    wxGCDC dc(paintDc);
    wxGraphicsContext* gc = dc.GetGraphicsContext();
    gc->SetAntialiasMode(wxANTIALIAS_NONE);
    gc->SetInterpolationQuality(wxINTERPOLATION_NONE);

    xOffset = FRAME_IMAGE_OFFSET - xOffset;
    yOffset = FRAME_IMAGE_OFFSET - yOffset;

    // Origin
    // ------
    {
        dc.SetPen(DC::ORIGIN_PEN);
        dc.CrossHair(xOffset * zoomX, yOffset * zoomY);
    }

    // Boxes
    // -----
    dc.SetBrush(wxNullBrush);
    dc.SetPen(DC::FRAME_OBJECTS_PEN);

    for (const MS::FrameObject& obj : frame->objects()) {
        const auto oLoc = obj.location();
        const unsigned oSize = obj.sizePx();

        DC::DrawRectangleOffset(dc, zoomX, zoomY, xOffset, yOffset,
                                oLoc.x, oLoc.y, oSize, oSize);
    }

    dc.SetBrush(DC::ENTITY_HITBOX_BRUSH);
    dc.SetPen(DC::ENTITY_HITBOX_PEN);
    for (const MS::EntityHitbox& eh : frame->entityHitboxes()) {
        const auto aabb = eh.aabb();

        DC::DrawRectangleOffset(dc, zoomX, zoomY, xOffset, yOffset,
                                aabb.x, aabb.y, aabb.width, aabb.height);
    }

    if (frame->solid()) {
        const auto aabb = frame->tileHitbox();

        dc.SetBrush(DC::TILE_HITBOX_BRUSH);
        dc.SetPen(DC::TILE_HITBOX_PEN);
        DC::DrawRectangleOffset(dc, zoomX, zoomY, xOffset, yOffset,
                                aabb.x, aabb.y, aabb.width, aabb.height);
    }

    dc.SetBrush(wxNullBrush);
    dc.SetPen(DC::ACTION_POINT_PEN);
    for (const MS::ActionPoint& ap : frame->actionPoints()) {
        const auto aLoc = ap.location();

        DC::DrawCrossOffset(dc, zoomX, zoomY, xOffset, yOffset,
                            aLoc.x, aLoc.y);
    }

    // Selected Item
    // -------------
    switch (_controller.selectedTypeController().type()) {
    case SelectedType::NONE:
        break;

    case SelectedType::FRAME_OBJECT:
        if (const auto* fo = _controller.frameObjectController().selected()) {
            const auto oLoc = fo->location();
            const unsigned oSize = fo->sizePx();

            DC::DrawSelectedRectangleOffset(dc, zoomX, zoomY, xOffset, yOffset,
                                            oLoc.x, oLoc.y, oSize, oSize);
        }
        break;

    case SelectedType::ENTITY_HITBOX:
        if (const auto* eh = _controller.entityHitboxController().selected()) {
            const auto aabb = eh->aabb();

            DC::DrawSelectedRectangleOffset(dc, zoomX, zoomY, xOffset, yOffset,
                                            aabb.x, aabb.y, aabb.width, aabb.height);
        }
        break;

    case SelectedType::ACTION_POINT:
        if (const auto* ap = _controller.actionPointController().selected()) {
            const auto aLoc = ap->location();

            DC::DrawSelectedCrossOffset(dc, zoomX, zoomY, xOffset, yOffset,
                                        aLoc.x, aLoc.y);
        }
        break;
    }
}
