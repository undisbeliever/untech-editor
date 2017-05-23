/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tileset.h"
#include "tile.hpp"
#include <cstring>

namespace UnTech {
namespace Snes {
namespace Private {

template <size_t BD>
inline void writeSnesTile8px(uint8_t out[8 * BD], const Tile<8>& tile)
{
    constexpr unsigned TILE_SIZE = 8;
    constexpr unsigned BIT_DEPTH = BD;
    constexpr unsigned SNES_DATA_SIZE = 8 * BD;

    unsigned pos = 0;
    memset(out, 0, SNES_DATA_SIZE);

    for (unsigned b = 0; b < BIT_DEPTH; b += 2) {
        for (unsigned y = 0; y < TILE_SIZE; y++) {
            const uint8_t* tileRow = tile.rawData() + y * 8;

            const unsigned biEnd = (b < BIT_DEPTH - 1) ? 2 : 1;
            for (unsigned bi = 0; bi < biEnd; bi++) {
                uint_fast8_t byte = 0;
                uint_fast8_t mask = 1 << (b + bi);

                for (unsigned x = 0; x < TILE_SIZE; x++) {
                    byte <<= 1;

                    if (tileRow[x] & mask)
                        byte |= 1;
                }
                out[pos++] = byte;
            }
        }
    }
}

template <size_t BD>
inline void readSnesTile8px(Tile<8>& tile, const uint8_t data[8 * BD])
{
    constexpr unsigned TILE_SIZE = 8;
    constexpr unsigned BIT_DEPTH = BD;

    const uint8_t* dataPos = data;

    memset(tile.rawData(), 0, tile.TILE_ARRAY_SIZE);

    for (unsigned b = 0; b < BIT_DEPTH; b += 2) {
        for (unsigned y = 0; y < TILE_SIZE; y++) {
            uint8_t* tileRow = tile.rawData() + y * 8;

            const unsigned biEnd = (b < BIT_DEPTH - 1) ? 2 : 1;
            for (unsigned bi = 0; bi < biEnd; bi++) {
                uint_fast8_t byte = *dataPos++;
                uint_fast8_t bits = 1 << (b + bi);

                for (unsigned x = 0; x < TILE_SIZE; x++) {
                    if (byte & 0x80) {
                        tileRow[x] |= bits;
                    }
                    byte <<= 1;
                }
            }
        }
    }
}
}
}
}

using namespace UnTech::Snes;
using namespace UnTech::Snes::Private;

template <size_t BD>
void Tileset8px<BD>::drawTile(Image& image, const Palette<BD>& palette,
                              unsigned xOffset, unsigned yOffset,
                              unsigned tileId, const bool hFlip, const bool vFlip) const
{
    if (_tiles.size() <= tileId) {
        return;
    }

    _tiles[tileId].draw(image, palette, xOffset, yOffset, hFlip, vFlip);
}

template <size_t BD>
inline std::vector<uint8_t> Tileset8px<BD>::snesData() const
{
    std::vector<uint8_t> out(SNES_TILE_SIZE * _tiles.size());
    uint8_t* outData = out.data();

    for (const auto& tile : _tiles) {
        writeSnesTile8px<BIT_DEPTH>(outData, tile);

        outData += SNES_TILE_SIZE;
    }

    return out;
}

template <size_t BD>
inline void Tileset8px<BD>::readSnesData(const std::vector<uint8_t>& in)
{
    const uint8_t* inData = in.data();
    size_t toAdd = in.size() / SNES_TILE_SIZE;

    size_t oldSize = _tiles.size();
    _tiles.resize(oldSize + toAdd);

    for (size_t i = 0; i < toAdd; i++) {
        TileT& tile = _tiles[i + oldSize];
        readSnesTile8px<BIT_DEPTH>(tile, inData);

        inData += SNES_TILE_SIZE;
    }
}

template class UnTech::Snes::Tileset8px<1>;
template class UnTech::Snes::Tileset8px<2>;
template class UnTech::Snes::Tileset8px<3>;
template class UnTech::Snes::Tileset8px<4>;
template class UnTech::Snes::Tileset8px<8>;

void TilesetTile16::drawTile(Image& image, const Palette<4>& palette,
                             unsigned xOffset, unsigned yOffset,
                             unsigned tileId, const bool hFlip, const bool vFlip) const
{
    if (_tiles.size() <= tileId) {
        return;
    }

    _tiles[tileId].draw(image, palette, xOffset, yOffset, hFlip, vFlip);
}

std::vector<uint8_t> TilesetTile16::snesData() const
{
    std::vector<uint8_t> out(SNES_TILE_SIZE * _tiles.size());
    uint8_t* outData = out.data();

    for (const Tile<16>& tile : _tiles) {
        auto smallTiles = splitLargeTile(tile);

        for (const Tile<8>& st : smallTiles) {
            writeSnesTile8px<BIT_DEPTH>(outData, st);

            outData += SNES_SMALL_TILE_SIZE;
        }
    }

    return out;
}

void TilesetTile16::readSnesData(const std::vector<uint8_t>& in)
{
    const uint8_t* inData = in.data();
    size_t toAdd = in.size() / SNES_TILE_SIZE;

    size_t oldSize = _tiles.size();
    _tiles.resize(oldSize + toAdd);

    for (size_t i = 0; i < toAdd; i++) {
        std::array<Tile<8>, 4> smallTiles;

        for (Tile<8>& tile8 : smallTiles) {
            readSnesTile8px<BIT_DEPTH>(tile8, inData);
            inData += SNES_SMALL_TILE_SIZE;
        }

        _tiles[i + oldSize] = combineSmallTiles(smallTiles);
    }
}
