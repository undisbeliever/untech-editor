#pragma once

#include "models/common/indexedimage.h"
#include "models/snes/snescolor.h"
#include "models/snes/tilemap.h"
#include "models/snes/tileset.h"

#include <vector>

namespace UnTech {
namespace Snes {

template <size_t BIT_DEPTH>
class Image2Snes {
public:
    using TileT = Tile8px<BIT_DEPTH>;
    constexpr static unsigned COLORS_PER_PALETTE = 1 << BIT_DEPTH;

public:
    Image2Snes() = default;

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

    Tileset<TileT> _tileset;
    std::vector<SnesColor> _palette;
    Tilemap _tilemap;
};
}
}
