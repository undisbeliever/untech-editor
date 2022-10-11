/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "bit-depth.h"
#include "models/common/indexedimage.h"
#include "models/snes/snescolor.h"
#include "models/snes/tile-data.h"
#include "models/snes/tile.h"
#include "models/snes/tilemap.h"

#include <vector>

namespace UnTech::Snes {

class Image2Snes {

private:
    unsigned _tileOffset = 0;
    unsigned _maxTiles = 1024;
    unsigned _paletteOffset = 0;
    unsigned _maxPalettes = 8;
    bool _order = 0;

    const Snes::BitDepthSpecial _bitDepth;
    std::vector<Tile8px> _tileset;
    std::vector<SnesColor> _palette;
    Tilemap _tilemap;

public:
    Image2Snes(BitDepthSpecial bd);

    void setTileOffset(unsigned o) { _tileOffset = o; }
    void setMaxTiles(unsigned t) { _maxTiles = t; }
    void setPaletteOffset(unsigned o) { _paletteOffset = o; }
    void setMaxPalettes(unsigned m) { _maxPalettes = m; }
    void setOrder(bool o) { _order = o; }

    [[nodiscard]] BitDepthSpecial bitDepth() const { return _bitDepth; }
    [[nodiscard]] const auto& tileset() const { return _tileset; }
    [[nodiscard]] const auto& palette() const { return _palette; }
    [[nodiscard]] const auto& tilemap() const { return _tilemap; }

    [[nodiscard]] std::vector<uint8_t> tilesetSnesData() const { return Snes::snesTileData(_tileset, _bitDepth); };
    [[nodiscard]] std::vector<uint8_t> paletteSnesData() const;

    void process(const IndexedImage& image);
};

}
