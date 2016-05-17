#ifndef _UNTECH_MODELS_SNES_TILESET_H_
#define _UNTECH_MODELS_SNES_TILESET_H_

#include "palette.h"
#include "tile.h"
#include "../common/image.h"
#include <array>
#include <cstdint>
#include <vector>

namespace UnTech {
namespace Snes {

// ::SHOULDO draw large tile (8, 16, 32) function::

template <class TileT>
class Tileset {
public:
    typedef TileT tile_t;

public:
    // fails silently
    void drawTile(Image& image, const Palette<TileT::BIT_DEPTH>& palette,
                  unsigned xOffset, unsigned yOffset,
                  unsigned tileId, bool hFlip = false, bool vFlip = false) const;

    void addTile() { _tiles.emplace_back(); }
    void addTile(const TileT& tile) { _tiles.emplace_back(tile); }

    TileT& tile(size_t n) { return _tiles.at(n); }
    const TileT& tile(size_t n) const { return _tiles.at(n); }

    std::vector<uint8_t> snesData() const;
    void readSnesData(const std::vector<uint8_t>& data);

    // expose vector
    size_t size() const { return _tiles.size(); }
    auto begin() { return _tiles.begin(); }
    auto begin() const { return _tiles.begin(); }
    auto end() { return _tiles.begin(); }
    auto end() const { return _tiles.begin(); }

protected:
    std::vector<TileT> _tiles;
};

typedef Tileset<Tile2bpp8px> Tileset2bpp8px;
typedef Tileset<Tile4bpp8px> Tileset4bpp8px;
typedef Tileset<Tile8bpp8px> Tileset8bpp8px;

typedef Tileset<Tile4bpp16px> Tileset4bpp16px;
}
}
#endif
