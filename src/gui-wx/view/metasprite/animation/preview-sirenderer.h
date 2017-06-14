/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once
#include "previewpanel.h"
#include "gui-wx/controllers/metasprite/spriteimporter.h"
#include <wx/rawbmp.h>

namespace UnTech {
namespace View {
namespace MetaSprite {
namespace Animation {

namespace SI = UnTech::MetaSprite::SpriteImporter;
namespace MSA = UnTech::MetaSprite::Animation;

class PreviewSiRenderer : public AbstractPreviewRenderer {
    constexpr static unsigned BITMAP_SIZE = 256;

public:
    PreviewSiRenderer(SI::SpriteImporterController& controller);

    virtual void Render(wxPaintDC& dc, const MSA::PreviewState& state) final;

private:
    void UpdateBitmap(const UnTech::MetaSprite::NameReference& frame);

    void DrawTile(wxNativePixelData& pixelData,
                  const SI::FrameSet& frameSet,
                  const unsigned bitmapX, const unsigned bitmapY,
                  const unsigned tileX, const unsigned tileY,
                  const unsigned tileSize,
                  bool hFlip, bool vFlip);

private:
    SI::SpriteImporterController& _controller;
    wxBitmap _bitmap;
};
}
}
}
}
