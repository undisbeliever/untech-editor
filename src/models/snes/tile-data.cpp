/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tile-data.h"
#include "tile.h"
#include "models/common/iterators.h"
#include <cstring>

namespace UnTech::Snes {

constexpr static inline unsigned tileOffset(const unsigned bitDepth, const unsigned bitPlane, const unsigned y)
{
    constexpr unsigned TILE_SIZE = 8;
    constexpr unsigned BITPLANE_PAIR_OFFSET = TILE_SIZE * 2;

    if (bitDepth % 2 == 0 || bitPlane != (bitDepth - 1)) {
        return y * 2 + (bitPlane / 2) * BITPLANE_PAIR_OFFSET + (bitPlane & 1);
    }
    else {
        // If the bitDepth is odd then the last bitplane is in a different location
        const unsigned LAST_BITPLANE_OFFSET = (bitDepth - 1) * TILE_SIZE * 2;

        return LAST_BITPLANE_OFFSET + y;
    }
}

template <unsigned BIT_DEPTH>
static inline uint8_t* writeSnesTile(uint8_t* out, const Tile8px& tile)
{
    constexpr unsigned TILE_SIZE = 8;
    constexpr unsigned TILE_DATA_SIZE = TILE_SIZE * TILE_SIZE * BIT_DEPTH / 8;

    static_assert(BIT_DEPTH == 8 || (BIT_DEPTH >= 1 && BIT_DEPTH <= 4));

    for (const auto y : range(TILE_SIZE)) {
        const uint8_t* const sliver = tile.rawData() + y * TILE_SIZE;

        // Force loop unrolling (GCC -O2 does not unroll `writeBitRow` if I place it inside a for loop)
        auto writeBitRow = [&](const unsigned b) {
            uint8_t byte = 0;

            for (const auto x : range(TILE_SIZE)) {
                byte <<= 1;
                byte |= bool(sliver[x] & (1 << b));
            }

            out[tileOffset(BIT_DEPTH, b, y)] = byte;
        };

        if constexpr (BIT_DEPTH >= 1) {
            writeBitRow(0);
        }
        if constexpr (BIT_DEPTH >= 2) {
            writeBitRow(1);
        }
        if constexpr (BIT_DEPTH >= 3) {
            writeBitRow(2);
        }
        if constexpr (BIT_DEPTH >= 4) {
            writeBitRow(3);
        }
        if constexpr (BIT_DEPTH == 8) {
            writeBitRow(7);
            writeBitRow(6);
            writeBitRow(5);
            writeBitRow(4);
        }
    }

    return out + TILE_DATA_SIZE;
}

template <unsigned BIT_DEPTH>
static inline const uint8_t* readSnesTile(Tile8px& tile, const uint8_t* input)
{
    constexpr unsigned TILE_SIZE = 8;
    constexpr unsigned TILE_DATA_SIZE = TILE_SIZE * TILE_SIZE * BIT_DEPTH / 8;

    static_assert(BIT_DEPTH == 8 || (BIT_DEPTH >= 1 && BIT_DEPTH <= 4));

    auto tileIt = tile.rawData();

    for (const auto y : range(TILE_SIZE)) {
        for (const auto x : range(TILE_SIZE)) {
            uint8_t pixel = 0;

            // Force loop unrolling (GCC -O2 does not unroll `readBit` if I place it inside a for loop)
            auto readBitRow = [&](const unsigned b) {
                pixel <<= 1;
                pixel |= bool(input[tileOffset(BIT_DEPTH, b, y)] & (0x80 >> x));
            };

            if constexpr (BIT_DEPTH == 8) {
                readBitRow(7);
                readBitRow(6);
                readBitRow(5);
                readBitRow(4);
            }
            if constexpr (BIT_DEPTH >= 4) {
                readBitRow(3);
            }
            if constexpr (BIT_DEPTH >= 3) {
                readBitRow(2);
            }
            if constexpr (BIT_DEPTH >= 2) {
                readBitRow(1);
            }
            if constexpr (BIT_DEPTH >= 1) {
                readBitRow(0);
            }

            *tileIt++ = pixel;
        }
    }
    assert(tileIt == tile.rawData() + tile.data().size());

    return input + TILE_DATA_SIZE;
}

template <unsigned BIT_DEPTH>
static inline std::vector<uint8_t> tilesToSnesData(const std::vector<Tile8px>& tiles)
{
    constexpr unsigned TILE_SIZE = 8;
    constexpr unsigned TILE_DATA_SIZE = TILE_SIZE * TILE_SIZE * BIT_DEPTH / 8;

    std::vector<uint8_t> out(tiles.size() * TILE_DATA_SIZE);

    uint8_t* outIt = out.data();

    for (const auto& tile : tiles) {
        outIt = writeSnesTile<BIT_DEPTH>(outIt, tile);
    }
    assert(outIt == out.data() + out.size());

    return out;
}

std::vector<uint8_t> snesTileData4bppTile16(const std::vector<Tile16px>& tileset)
{
    constexpr unsigned TILE_SIZE = 16;
    constexpr unsigned BIT_DEPTH = 4;
    constexpr unsigned TILE_DATA_SIZE = TILE_SIZE * TILE_SIZE * BIT_DEPTH / 8;

    std::vector<uint8_t> out(TILE_DATA_SIZE * tileset.size());
    uint8_t* outIt = out.data();

    for (const Tile16px& tile : tileset) {
        const auto smallTiles = splitLargeTile(tile);

        for (const Tile8px& subTile : smallTiles) {
            outIt = writeSnesTile<BIT_DEPTH>(outIt, subTile);
        }
    }
    assert(outIt == out.data() + out.size());

    return out;
}

template <unsigned BIT_DEPTH>
static inline std::vector<Tile8px> snesDataToTiles(const std::vector<uint8_t>& in)
{
    constexpr unsigned TILE_SIZE = 8;
    constexpr unsigned TILE_DATA_SIZE = TILE_SIZE * TILE_SIZE * BIT_DEPTH / 8;

    if (in.size() % TILE_DATA_SIZE != 0) {
        throw std::runtime_error("snesDataToTiles: invalid data size");
    }

    std::vector<Tile8px> out(in.size() / TILE_DATA_SIZE);

    const uint8_t* inIt = in.data();

    for (Tile8px& tile : out) {
        inIt = readSnesTile<BIT_DEPTH>(tile, inIt);
    }

    assert(inIt == in.data() + in.size());

    return out;
}

std::vector<Tile16px> readSnesTileData4bppTile16(const std::vector<uint8_t>& in)
{
    constexpr unsigned TILE_SIZE = 16;
    constexpr unsigned BIT_DEPTH = 4;
    constexpr unsigned TILE_DATA_SIZE = TILE_SIZE * TILE_SIZE * BIT_DEPTH / 8;

    if (in.size() % TILE_DATA_SIZE != 0) {
        throw std::runtime_error("readSnesData4bppTile16: invalid data size");
    }

    std::vector<Tile16px> out(in.size() / TILE_DATA_SIZE);

    const uint8_t* inIt = in.data();

    for (Tile16px& tile : out) {
        std::array<Tile8px, 4> smallTiles;

        for (Tile8px& tile8 : smallTiles) {
            inIt = readSnesTile<BIT_DEPTH>(tile8, inIt);
        }

        tile = combineSmallTiles(smallTiles);
    }

    assert(inIt == in.data() + in.size());

    return out;
}

std::vector<uint8_t> snesTileData1bpp(const std::vector<Tile8px>& tiles)
{
    return tilesToSnesData<1>(tiles);
}

std::vector<uint8_t> snesTileData2bpp(const std::vector<Tile8px>& tiles)
{
    return tilesToSnesData<2>(tiles);
}

std::vector<uint8_t> snesTileData3bpp(const std::vector<Tile8px>& tiles)
{
    return tilesToSnesData<3>(tiles);
}

std::vector<uint8_t> snesTileData4bpp(const std::vector<Tile8px>& tiles)
{
    return tilesToSnesData<4>(tiles);
}

std::vector<uint8_t> snesTileData8bpp(const std::vector<Tile8px>& tiles)
{
    return tilesToSnesData<8>(tiles);
}

std::vector<uint8_t> snesTileData(const std::vector<Tile8px>& tiles, const unsigned bitDepth)
{
    switch (bitDepth) {
    case 1:
        return snesTileData1bpp(tiles);

    case 2:
        return snesTileData2bpp(tiles);

    case 3:
        return snesTileData3bpp(tiles);

    case 4:
        return snesTileData4bpp(tiles);

    case 8:
        return snesTileData8bpp(tiles);
    }

    throw std::runtime_error("Invalid bitdepth");
}

std::vector<Tile8px> readSnesTileData1bpp(const std::vector<uint8_t>& in)
{
    return snesDataToTiles<1>(in);
}

std::vector<Tile8px> readSnesTileData2bpp(const std::vector<uint8_t>& in)
{
    return snesDataToTiles<2>(in);
}

std::vector<Tile8px> readSnesTileData3bpp(const std::vector<uint8_t>& in)
{
    return snesDataToTiles<3>(in);
}

std::vector<Tile8px> readSnesTileData4bpp(const std::vector<uint8_t>& in)
{
    return snesDataToTiles<4>(in);
}

std::vector<Tile8px> readSnesTileData8bpp(const std::vector<uint8_t>& in)
{
    return snesDataToTiles<8>(in);
}

std::vector<Tile8px> readSnesTileData(const std::vector<uint8_t>& in, const unsigned bitDepth)
{
    switch (bitDepth) {
    case 1:
        return readSnesTileData1bpp(in);

    case 2:
        return readSnesTileData2bpp(in);

    case 3:
        return readSnesTileData3bpp(in);

    case 4:
        return readSnesTileData4bpp(in);

    case 8:
        return readSnesTileData8bpp(in);
    }

    throw std::runtime_error("Invalid bitdepth");
}

}
