/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/indexedimage.h"
#include "models/snes/snescolor.h"
#include "models/snes/tilemap.h"
#include "models/snes/tileset.h"

#include <vector>

namespace UnTech {
namespace Snes {

class Image2Snes {
public:
    Image2Snes(int bitDepth);

    void setTileOffset(unsigned o) { _tileOffset = o; }
    void setMaxTiles(unsigned t) { _maxTiles = t; }
    void setPaletteOffset(unsigned o) { _paletteOffset = o; }
    void setMaxPalettes(unsigned m) { _maxPalettes = m; }
    void setOrder(bool o) { _order = o; }

    const auto& tileset() const { return _tileset; }
    const auto& palette() const { return _palette; }
    const auto& tilemap() const { return _tilemap; }

    std::vector<uint8_t> paletteSnesData() const;

    void process(const IndexedImage& image);

private:
    unsigned _tileOffset = 0;
    unsigned _maxTiles = 1024;
    unsigned _paletteOffset = 0;
    unsigned _maxPalettes = 8;
    bool _order = 0;

    Tileset8px _tileset;
    std::vector<SnesColor> _palette;
    Tilemap _tilemap;
};
}
}
