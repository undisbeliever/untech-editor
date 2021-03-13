/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tileset.h"
#include "models/common/iterators.h"
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
        for (const auto y : range(TILE_SIZE)) {
            const uint8_t* tileRow = tile.rawData() + y * 8;

            const unsigned biEnd = (b < bitDepth - 1) ? 2 : 1;
            for (const auto bi : range(biEnd)) {
                uint_fast8_t byte = 0;
                uint_fast8_t mask = 1 << (b + bi);

                for (const auto x : range(TILE_SIZE)) {
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
        for (const auto y : range(TILE_SIZE)) {
            uint8_t* tileRow = tile.rawData() + y * 8;

            const unsigned biEnd = (b < bitDepth - 1) ? 2 : 1;
            for (const auto bi : range(biEnd)) {
                unsigned byte = *dataPos++;
                unsigned bits = 1 << (b + bi);

                for (const auto x : range(TILE_SIZE)) {
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
    const size_t toAdd = in.size() / snesTileSize;

    const size_t oldSize = _tiles.size();
    _tiles.resize(oldSize + toAdd);

    for (const auto i : range(toAdd)) {
        TileT& tile = _tiles[i + oldSize];
        readSnesTile8px(tile, inData, unsigned(_bitDepth));

        inData += snesTileSize;
    }

    assert(inData == in.data() + toAdd * snesTileSize);
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
    const size_t toAdd = in.size() / SNES_TILE_SIZE;

    const size_t oldSize = _tiles.size();
    _tiles.resize(oldSize + toAdd);

    for (const auto i : range(toAdd)) {
        std::array<Tile<8>, 4> smallTiles;

        for (Tile<8>& tile8 : smallTiles) {
            readSnesTile8px(tile8, inData, BIT_DEPTH);
            inData += SNES_SMALL_TILE_SIZE;
        }

        _tiles[i + oldSize] = combineSmallTiles(smallTiles);
    }
}
