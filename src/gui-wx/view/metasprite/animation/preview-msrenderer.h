/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once
#include "previewpanel.h"
#include "gui-wx/controllers/metasprite/metasprite.h"

namespace UnTech {
namespace View {
namespace MetaSprite {
namespace Animation {

namespace MS = UnTech::MetaSprite::MetaSprite;
namespace MSA = UnTech::MetaSprite::Animation;

class PreviewMsRenderer : public AbstractPreviewRenderer {
    constexpr static int BITMAP_SIZE = 256;

public:
    PreviewMsRenderer(MS::MetaSpriteController& controller);

    virtual void Render(wxPaintDC& dc, const MSA::PreviewState& state) final;

private:
    void UpdateBitmap(const UnTech::MetaSprite::NameReference& frame);

private:
    MS::MetaSpriteController& _controller;
    wxBitmap _bitmap;
};
}
}
}
}
