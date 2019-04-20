/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tileset.h"
#include "tile.hpp"
#include <cstring>

namespace UnTech {
namespace Snes {
namespace Private {

static void writeSnesTile8px(uint8_t* out, const Tile<8>& tile, const unsigned bitDepth)
{
    constexpr unsigned TILE_SIZE = 8;
    assert(bitDepth <= 8);

    unsigned pos = 0;

    for (unsigned b = 0; b < bitDepth; b += 2) {
        for (unsigned y = 0; y < TILE_SIZE; y++) {
            const uint8_t* tileRow = tile.rawData() + y * 8;

            const unsigned biEnd = (b < bitDepth - 1) ? 2 : 1;
            for (unsigned bi = 0; bi < biEnd; bi++) {
                uint_fast8_t byte = 0;
                uint_fast8_t mask = 1 << (b + bi);

                for (unsigned x = 0; x < TILE_SIZE; x++) {
                    byte <<= 1;

                    if (tileRow[x] & mask) {
                        byte |= 1;
                    }
                }
                out[pos++] = byte;
            }
        }
    }
}

static void readSnesTile8px(Tile<8>& tile, const uint8_t* data, const unsigned bitDepth)
{
    constexpr unsigned TILE_SIZE = 8;
    assert(bitDepth <= 8);

    const uint8_t* dataPos = data;

    memset(tile.rawData(), 0, tile.TILE_ARRAY_SIZE);

    for (unsigned b = 0; b < bitDepth; b += 2) {
        for (unsigned y = 0; y < TILE_SIZE; y++) {
            uint8_t* tileRow = tile.rawData() + y * 8;

            const unsigned biEnd = (b < bitDepth - 1) ? 2 : 1;
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

Tileset8px::BitDepth Tileset8px::depthFromInt(int bd)
{
    switch (bd) {
    case 1:
        return BitDepth::BD_1BPP;

    case 2:
        return BitDepth::BD_2BPP;

    case 3:
        return BitDepth::BD_3BPP;

    case 4:
        return BitDepth::BD_4BPP;

    case 8:
        return BitDepth::BD_8BPP;
    }

    throw std::invalid_argument("Unknown bit-depth, expected 1, 2, 3, 4 or 8");
}

template <size_t BD>
void Tileset8px::drawTile(Image& image, const Palette<BD>& palette,
                          unsigned xOffset, unsigned yOffset,
                          unsigned tileId, const bool hFlip, const bool vFlip) const
{
    if (_tiles.size() <= tileId) {
        return;
    }

    _tiles[tileId].draw(image, palette, xOffset, yOffset, hFlip, vFlip);
}

template void Tileset8px::drawTile<2>(Image&, const Palette<2>&, unsigned, unsigned, unsigned, const bool, const bool) const;
template void Tileset8px::drawTile<4>(Image&, const Palette<4>&, unsigned, unsigned, unsigned, const bool, const bool) const;
template void Tileset8px::drawTile<8>(Image&, const Palette<8>&, unsigned, unsigned, unsigned, const bool, const bool) const;

std::vector<uint8_t> Tileset8px::snesData() const
{
    const unsigned snesTileSize = this->snesTileSize();

    std::vector<uint8_t> out(snesTileSize * _tiles.size());
    uint8_t* outData = out.data();

    for (const auto& tile : _tiles) {
        writeSnesTile8px(outData, tile, unsigned(_bitDepth));

        outData += snesTileSize;
    }

    assert(outData == out.data() + out.size());

    return out;
}

void Tileset8px::readSnesData(const std::vector<uint8_t>& in)
{
    const unsigned snesTileSize = this->snesTileSize();

    const uint8_t* inData = in.data();
    size_t toAdd = in.size() / snesTileSize;

    size_t oldSize = _tiles.size();
    _tiles.resize(oldSize + toAdd);

    for (size_t i = 0; i < toAdd; i++) {
        TileT& tile = _tiles[i + oldSize];
        readSnesTile8px(tile, inData, unsigned(_bitDepth));

        inData += snesTileSize;
    }

    assert(inData == in.data() + toAdd * snesTileSize);
}

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
            writeSnesTile8px(outData, st, BIT_DEPTH);

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
            readSnesTile8px(tile8, inData, BIT_DEPTH);
            inData += SNES_SMALL_TILE_SIZE;
        }

        _tiles[i + oldSize] = combineSmallTiles(smallTiles);
    }
}
