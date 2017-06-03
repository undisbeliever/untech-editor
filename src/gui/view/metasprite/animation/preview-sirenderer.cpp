/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "preview-sirenderer.h"
#include "gui/view/common/drawing.hpp"
#include <cassert>
#include <wx/dc.h>
#include <wx/dcgraph.h>

using namespace UnTech::View::MetaSprite::Animation;

PreviewSiRenderer::PreviewSiRenderer(SI::SpriteImporterController& controller)
    : _controller(controller)
    , _bitmap(BITMAP_SIZE, BITMAP_SIZE, 24)
{
}

void PreviewSiRenderer::Render(wxPaintDC& paintDc, const MSA::PreviewState& state)
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

    auto dcSize = paintDc.GetSize();
    const int xOffset = dcSize.GetWidth() / zoomX / 2;
    const int yOffset = dcSize.GetHeight() / zoomY / 2;

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

    const auto& frameOrigin = framePtr->location.origin;

    if (layers.entityHitboxes()) {
        dc.SetBrush(ENTITY_HITBOX_BRUSH);
        dc.SetPen(ENTITY_HITBOX_PEN);
        for (const SI::EntityHitbox& eh : framePtr->entityHitboxes) {
            auto r = ms8rect::createFromOffset(eh.aabb, frameOrigin);
            r = r.flip(frameRef.hFlip, frameRef.vFlip);
            helper.DrawRectangle(r);
        }
    }

    if (framePtr->solid && layers.tileHitbox()) {
        dc.SetBrush(TILE_HITBOX_BRUSH);
        dc.SetPen(TILE_HITBOX_PEN);

        auto r = ms8rect::createFromOffset(framePtr->tileHitbox, frameOrigin);
        r = r.flip(frameRef.hFlip, frameRef.vFlip);
        helper.DrawRectangle(r);
    }

    if (layers.actionPoints()) {
        dc.SetBrush(wxNullBrush);
        dc.SetPen(ACTION_POINT_PEN);
        for (const SI::ActionPoint& ap : framePtr->actionPoints) {
            auto p = ms8point::createFromOffset(ap.location, frameOrigin);
            p = p.flip(frameRef.hFlip, frameRef.vFlip);
            helper.DrawCross(p);
        }
    }
}

void PreviewSiRenderer::UpdateBitmap(const UnTech::MetaSprite::NameReference& frameRef)
{
    wxMemoryDC dc;
    dc.SelectObject(_bitmap);
    dc.SetBackground(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE)));
    dc.Clear();

    const auto* framePtr = _controller.frameSetController().selected().frames.getPtr(frameRef.name);
    if (framePtr == nullptr) {
        return;
    }

    const SI::FrameSet& frameSet = _controller.frameSetController().selected();
    if (frameSet.image == nullptr) {
        return;
    }

    const auto& fLoc = framePtr->location.aabb;
    const auto& origin = framePtr->location.origin;

    wxNativePixelData pData(_bitmap);

    for (const SI::FrameObject& obj : framePtr->objects) {
        unsigned bitmapX = obj.location.x - origin.x + BITMAP_SIZE / 2;
        unsigned bitmapY = obj.location.y - origin.y + BITMAP_SIZE / 2;
        unsigned tileX = fLoc.x + obj.location.x;
        unsigned tileY = fLoc.y + obj.location.y;

        DrawTile(pData, frameSet,
                 bitmapX, bitmapY,
                 tileX, tileY, obj.sizePx(),
                 frameRef.hFlip, frameRef.vFlip);
    }
}

void PreviewSiRenderer::DrawTile(wxNativePixelData& pixelData,
                                 const SI::FrameSet& frameSet,
                                 const unsigned bitmapX, const unsigned bitmapY,
                                 const unsigned tileX, const unsigned tileY,
                                 const unsigned tileSize,
                                 bool hFlip, bool vFlip)
{
    assert(frameSet.image != nullptr);
    const Image& fsImage = *frameSet.image;

    if (pixelData.GetWidth() < int(bitmapX + tileSize)
        || pixelData.GetHeight() < int(bitmapY + tileSize)
        || fsImage.size().width < (tileX + tileSize)
        || fsImage.size().height < (tileY + tileSize)) {

        return;
    }

    wxNativePixelData::Iterator imgBits(pixelData);
    wxNativePixelData::Iterator rowIt(pixelData);
    rowIt.Offset(pixelData, bitmapX, bitmapY);

    for (unsigned y = 0; y < tileSize; y++) {
        unsigned fy = (vFlip == false) ? y : tileSize - 1 - y;

        imgBits = rowIt;
        const rgba* tileRow = fsImage.scanline(tileY + fy) + tileX;

        for (unsigned x = 0; x < tileSize; x++) {
            unsigned fx = (hFlip == false) ? x : tileSize - 1 - x;

            rgba c = tileRow[fx];
            if (c != frameSet.transparentColor) {
                imgBits.Red() = c.red;
                imgBits.Green() = c.green;
                imgBits.Blue() = c.blue;
            }
            imgBits++;
        }
        rowIt.OffsetY(pixelData, 1);
    }
}
