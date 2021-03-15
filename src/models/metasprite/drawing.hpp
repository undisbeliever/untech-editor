/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "metasprite.h"
#include "models/snes/drawing.hpp"

namespace UnTech::MetaSprite::MetaSprite {

inline void drawFrame(Image& image,
                      const FrameSet& frameSet, const std::array<rgba, 16>& palette,
                      const Frame& frame, unsigned xOffset, unsigned yOffset)
{
    for (const auto& obj : reverse(frame.objects)) {
        if (obj.size == ObjectSize::SMALL) {
            Snes::drawTile_transparent(frameSet.smallTileset, obj.tileId, obj.hFlip, obj.vFlip,
                                       palette, image,
                                       xOffset + obj.location.x, yOffset + obj.location.y);
        }
        else {
            Snes::drawTile_transparent(frameSet.largeTileset, obj.tileId, obj.hFlip, obj.vFlip,
                                       palette, image,
                                       xOffset + obj.location.x, yOffset + obj.location.y);
        }
    }
}

}
