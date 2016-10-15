#include "tileset.h"
#include <cstring>

using namespace UnTech;
using namespace UnTech::Snes;

template <class TileT>
void Tileset<TileT>::drawTile(Image& image,
                              const Palette<TileT::BIT_DEPTH>& palette,
                              unsigned xOffset, unsigned yOffset,
                              unsigned tileId, const bool hFlip, const bool vFlip) const
{
    if (_tiles.size() <= tileId) {
        return;
    }

    _tiles[tileId].draw(image, palette, xOffset, yOffset, hFlip, vFlip);
}

template <class TileT>
inline std::vector<uint8_t> Tileset<TileT>::snesData() const
{
    std::vector<uint8_t> out(TileT::SNES_DATA_SIZE * _tiles.size());
    uint8_t* outData = out.data();

    for (const auto& tile : _tiles) {
        tile.writeSnesData(outData);

        outData += TileT::SNES_DATA_SIZE;
    }

    return out;
}

template <class TileT>
inline void Tileset<TileT>::readSnesData(const std::vector<uint8_t>& in)
{
    const uint8_t* inData = in.data();
    size_t count = in.size() / TileT::SNES_DATA_SIZE;

    for (size_t i = 0; i < count; i++) {
        _tiles.emplace_back(inData);

        inData += TileT::SNES_DATA_SIZE;
    }
}

template class Snes::Tileset<Tile2bpp8px>;
template class Snes::Tileset<Tile4bpp8px>;
template class Snes::Tileset<Tile8bpp8px>;
template class Snes::Tileset<Tile4bpp16px>;
