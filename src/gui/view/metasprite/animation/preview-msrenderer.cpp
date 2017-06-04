/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "preview-msrenderer.h"
#include "gui/view/common/drawing.hpp"
#include "gui/view/snes/tile.hpp"
#include <cassert>
#include <wx/dc.h>
#include <wx/dcgraph.h>

using namespace UnTech::View::MetaSprite::Animation;

PreviewMsRenderer::PreviewMsRenderer(MS::MetaSpriteController& controller)
    : _controller(controller)
    , _bitmap(BITMAP_SIZE, BITMAP_SIZE, 24)
{
}

void PreviewMsRenderer::Render(wxPaintDC& paintDc, const MSA::PreviewState& state)
{
    assert(paintDc.IsOk());

    UpdateBitmap(state.frame());

    const auto& frameRef = state.frame();
    const auto* framePtr = _controller.frameSetController().selected().frames.getPtr(frameRef.name);
    if (framePtr == nullptr) {
        return;
    }

    const auto& layers = _controller.settingsController().layers();
    const double zoomX = _controller.settingsController().zoom().zoomX();
    const double zoomY = _controller.settingsController().zoom().zoomY();

    wxSize dcSize = paintDc.GetSize();
    const point aPos = state.positionInt();
    const int xOffset = dcSize.GetWidth() / zoomX / 2 + aPos.x;
    const int yOffset = dcSize.GetHeight() / zoomY / 2 + aPos.y;

    if (_bitmap.IsOk()) {
        wxMemoryDC tmpDc;
        tmpDc.SelectObjectAsSource(_bitmap);

        int x = BITMAP_SIZE / 2 - xOffset;
        int y = BITMAP_SIZE / 2 - yOffset;
        int width = BITMAP_SIZE - x;
        int height = BITMAP_SIZE - y;

        if (width > 0 && height > 0) {
            paintDc.StretchBlit(0, 0, width * zoomX, height * zoomY,
                                &tmpDc,
                                x, y, width, height);
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

    DrawingHelper helper(dc, zoomX, zoomY, -xOffset, -yOffset);

    if (layers.entityHitboxes()) {
        dc.SetBrush(ENTITY_HITBOX_BRUSH);
        dc.SetPen(ENTITY_HITBOX_PEN);
        for (const MS::EntityHitbox& eh : framePtr->entityHitboxes) {
            auto r = eh.aabb.flip(frameRef.hFlip, frameRef.vFlip);
            helper.DrawRectangle(r);
        }
    }

    if (framePtr->solid && layers.tileHitbox()) {
        dc.SetBrush(TILE_HITBOX_BRUSH);
        dc.SetPen(TILE_HITBOX_PEN);

        auto r = framePtr->tileHitbox.flip(frameRef.hFlip, frameRef.vFlip);
        helper.DrawRectangle(r);
    }

    if (layers.actionPoints()) {
        dc.SetBrush(wxNullBrush);
        dc.SetPen(ACTION_POINT_PEN);
        for (const MS::ActionPoint& ap : framePtr->actionPoints) {
            auto p = ap.location.flip(frameRef.hFlip, frameRef.vFlip);
            helper.DrawCross(p);
        }
    }
}

void PreviewMsRenderer::UpdateBitmap(const UnTech::MetaSprite::NameReference& frameRef)
{
    typedef UnTech::MetaSprite::ObjectSize OS;

    wxMemoryDC dc;
    dc.SelectObject(_bitmap);
    dc.SetBackground(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE)));
    dc.Clear();

    const auto* framePtr = _controller.frameSetController().selected().frames.getPtr(frameRef.name);
    if (framePtr == nullptr) {
        return;
    }

    const MS::FrameSet& frameSet = _controller.frameSetController().selected();
    const auto& palette = _controller.paletteController().selected();

    wxNativePixelData pData(_bitmap);

    for (const MS::FrameObject& obj : framePtr->objects) {
        int xPos = obj.location.x;
        if (frameRef.hFlip ^ obj.hFlip) {
            xPos = -xPos - obj.sizePx();
        }
        xPos += BITMAP_SIZE / 2;

        int yPos = obj.location.y;
        if (frameRef.vFlip ^ obj.vFlip) {
            yPos = -yPos - obj.sizePx();
        }
        yPos += BITMAP_SIZE / 2;

        if (obj.size == OS::SMALL) {
            const auto& smallTileset = frameSet.smallTileset;

            if (obj.tileId < smallTileset.size()) {
                UnTech::View::Snes::DrawTileTransparent(
                    pData,
                    smallTileset.tile(obj.tileId), palette,
                    xPos, yPos,
                    obj.hFlip ^ frameRef.hFlip, obj.vFlip ^ frameRef.vFlip);
            }
        }
        else {
            const auto& largeTileset = frameSet.largeTileset;

            if (obj.tileId < largeTileset.size()) {
                UnTech::View::Snes::DrawTileTransparent(
                    pData,
                    largeTileset.tile(obj.tileId), palette,
                    xPos, yPos,
                    obj.hFlip ^ frameRef.hFlip, obj.vFlip ^ frameRef.vFlip);
            }
        }
    }
}
