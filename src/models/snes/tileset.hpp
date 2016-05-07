#ifndef _UNTECH_MODELS_SNES_TILESET_HPP_
#define _UNTECH_MODELS_SNES_TILESET_HPP_

#include "tileset.h"
#include <cstring>

namespace UnTech {
namespace Snes {

inline void _Tileset__insertTile8intoTile16(const uint8_t tile8[64], uint8_t tile16[256],
                                            unsigned xPos, unsigned yPos)
{
    const uint8_t* t8 = tile8;
    uint8_t* t16 = tile16 + (yPos * 16 + xPos);

    for (unsigned y = 0; y < 8; y++) {
        memcpy(t16, t8, 8);
        t8 += 8;
        t16 += 16;
    }
}

inline void _Tileset__insertTile16intoTile8(const uint8_t tile16[256], uint8_t tile8[64],
                                            unsigned xPos, unsigned yPos)
{
    const uint8_t* t16 = tile16 + (yPos * 16 + xPos);
    uint8_t* t8 = tile8;

    for (unsigned y = 0; y < 8; y++) {
        memcpy(t8, t16, 8);
        t16 += 16;
        t8 += 8;
    }
}

template <size_t BIT_DEPTH>
void _Tileset__read8pxTile(const uint8_t* inData, uint8_t tile[64])
{
    const unsigned TILE_SIZE = 8;

    static_assert(BIT_DEPTH <= 8, "BIT_DEPTH is too high");
    static_assert((BIT_DEPTH & 1) == 0, "BIT_DEPTH must be a multiple of 2");

    const uint8_t* dataPos = inData;
    memset(tile, 0, Tileset<BIT_DEPTH, 8>::TILE_DATA_SIZE);

    for (unsigned b = 0; b < BIT_DEPTH; b += 2) {
        for (unsigned y = 0; y < TILE_SIZE; y++) {
            uint8_t* tileRow = tile + y * 8;

            for (unsigned bi = 0; bi < 2; bi++) {
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

template <size_t BIT_DEPTH>
void _Tileset__write8pxTile(const uint8_t tile[64], uint8_t* outData)
{
    const unsigned TILE_SIZE = 8;

    static_assert(BIT_DEPTH <= 8, "BIT_DEPTH is too high");
    static_assert((BIT_DEPTH & 1) == 0, "BIT_DEPTH must be a multiple of 2");

    unsigned pos = 0;
    memset(outData, 0, Tileset<BIT_DEPTH, 8>::SNES_DATA_SIZE);

    for (unsigned b = 0; b < BIT_DEPTH; b += 2) {
        for (unsigned y = 0; y < TILE_SIZE; y++) {
            const uint8_t* tileRow = tile + y * 8;

            for (unsigned bi = 0; bi < 2; bi++) {
                uint_fast8_t byte = 0;
                uint_fast8_t mask = 1 << (b + bi);

                for (unsigned x = 0; x < TILE_SIZE; x++) {
                    byte <<= 1;

                    if (tileRow[x] & mask)
                        byte |= 1;
                }
                outData[pos++] = byte;
            }
        }
    }
}

template <size_t BIT_DEPTH, size_t TILE_SIZE>
void Tileset<BIT_DEPTH, TILE_SIZE>::drawTile(Image& image, const Palette<BIT_DEPTH>& palette,
                                             unsigned xOffset, unsigned yOffset,
                                             unsigned tileId, const bool hFlip, const bool vFlip) const
{
    if (_tiles.size() <= tileId
        || image.size().width < (xOffset + TILE_SIZE)
        || image.size().height < (yOffset + TILE_SIZE)) {

        return;
    }

    // ::SHOULDO refactor and profile to see if faster::

    rgba* imgBits;
    const uint8_t* tilePos = _tiles[tileId].data();

    for (unsigned y = 0; y < TILE_SIZE; y++) {
        if (!hFlip) {
            imgBits = image.scanline(yOffset + y);
        }
        else {
            imgBits = image.scanline(yOffset + TILE_SIZE - y - 1);
        }

        if (!vFlip) {
            imgBits += xOffset;

            for (unsigned x = 0; x < TILE_SIZE; x++) {
                auto p = *tilePos & PIXEL_MASK;

                if (p != 0) {
                    *imgBits = palette.color(p).rgb();
                }
                imgBits++;
                tilePos++;
            }
        }
        else {
            imgBits += xOffset + TILE_SIZE - 1;

            for (unsigned x = 0; x < TILE_SIZE; x++) {
                auto p = *tilePos & PIXEL_MASK;

                if (p != 0) {
                    *imgBits = palette.color(p).rgb();
                }
                imgBits--;
                tilePos++;
            }
        }
    }
}

template <size_t BD, size_t TS>
inline typename Tileset<BD, TS>::tileData_t Tileset<BD, TS>::tileHFlip(size_t n) const
{
    Tileset<BD, TS>::tileData_t hFlip;
    uint8_t(*hData)[TS] = (uint8_t(*)[TS])hFlip.data();

    const auto pixelData = (uint8_t(*)[TS])_tiles.at(n).data();

    for (unsigned y = 0; y < TS; y++) {
        for (unsigned x = 0; x < TS; x++) {
            hData[y][x] = pixelData[y][TS - x - 1];
        }
    }
    return hFlip;
}

template <size_t BD, size_t TS>
inline typename Tileset<BD, TS>::tileData_t Tileset<BD, TS>::tileVFlip(size_t n) const
{
    Tileset<BD, TS>::tileData_t vFlip;
    uint8_t(*vData)[TS] = (uint8_t(*)[TS])vFlip.data();

    const auto pixelData = (uint8_t(*)[TS])_tiles.at(n).data();

    for (unsigned y = 0; y < TS; y++) {
        for (unsigned x = 0; x < TS; x++) {
            vData[y][x] = pixelData[TILE_SIZE - y - 1][x];
        }
    }
    return vFlip;
}

template <size_t BD, size_t TS>
inline typename Tileset<BD, TS>::tileData_t Tileset<BD, TS>::tileHVFlip(size_t n) const
{
    Tileset<BD, TS>::tileData_t hvFlip;
    uint8_t(*hvData)[TS] = (uint8_t(*)[TS])hvFlip.data();

    const auto pixelData = (uint8_t(*)[TS])_tiles.at(n).data();

    for (unsigned y = 0; y < TS; y++) {
        for (unsigned x = 0; x < TS; x++) {
            hvData[y][x] = pixelData[TILE_SIZE - y - 1][TILE_SIZE - x - 1];
        }
    }
    return hvFlip;
}

template <size_t BD, size_t TS>
typename Tileset<BD, TS>::tileData_t
Tileset<BD, TS>::tile(size_t tileId, bool hFlip, bool vFlip) const
{
    if (hFlip == false) {
        if (vFlip == false) {
            return tile(tileId);
        }
        else {
            return tileVFlip(tileId);
        }
    }
    else {
        if (vFlip == false) {
            return tileHFlip(tileId);
        }
        else {
            return tileHVFlip(tileId);
        }
    }
}

template <size_t BIT_DEPTH>
inline std::vector<uint8_t> Tileset8px<BIT_DEPTH>::snesData() const
{
    std::vector<uint8_t> out(Tileset8px::SNES_DATA_SIZE * this->_tiles.size());
    uint8_t* outData = out.data();

    for (const auto& tile : this->_tiles) {
        _Tileset__write8pxTile<BIT_DEPTH>(tile.data(), outData);

        outData += Tileset8px::SNES_DATA_SIZE;
    }

    return out;
}

template <size_t BIT_DEPTH>
inline std::vector<uint8_t> Tileset16px<BIT_DEPTH>::snesData() const
{
    const size_t SNES_8_DATA_SIZE = Tileset8px<BIT_DEPTH>::SNES_DATA_SIZE;

    std::vector<uint8_t> out(SNES_8_DATA_SIZE * 4 * this->_tiles.size());
    uint8_t* outData = out.data();

    uint8_t tile8[8 * 8];

    for (const auto& tile : this->_tiles) {
        const uint8_t* tile16 = tile.data();

        _Tileset__insertTile16intoTile8(tile16, tile8, 0, 0);
        _Tileset__write8pxTile<BIT_DEPTH>(tile8, outData);
        outData += SNES_8_DATA_SIZE;

        _Tileset__insertTile16intoTile8(tile16, tile8, 8, 0);
        _Tileset__write8pxTile<BIT_DEPTH>(tile8, outData);
        outData += SNES_8_DATA_SIZE;

        _Tileset__insertTile16intoTile8(tile16, tile8, 0, 8);
        _Tileset__write8pxTile<BIT_DEPTH>(tile8, outData);
        outData += SNES_8_DATA_SIZE;

        _Tileset__insertTile16intoTile8(tile16, tile8, 8, 8);
        _Tileset__write8pxTile<BIT_DEPTH>(tile8, outData);
        outData += SNES_8_DATA_SIZE;
    }

    return out;
}

template <size_t BIT_DEPTH>
inline void Tileset8px<BIT_DEPTH>::readSnesData(const std::vector<uint8_t>& in)
{
    const uint8_t* inData = in.data();
    size_t count = in.size() / Tileset8px::SNES_DATA_SIZE;

    for (size_t i = 0; i < count; i++) {
        this->addTile();
        uint8_t* tile = this->_tiles.back().data();

        _Tileset__read8pxTile<BIT_DEPTH>(inData, tile);

        inData += Tileset8px::SNES_DATA_SIZE;
    }
}

template <size_t BIT_DEPTH>
inline void Tileset16px<BIT_DEPTH>::readSnesData(const std::vector<uint8_t>& in)
{
    const size_t SNES_8_DATA_SIZE = Tileset8px<BIT_DEPTH>::SNES_DATA_SIZE;

    const uint8_t* inData = in.data();
    size_t count = in.size() / Tileset16px::SNES_DATA_SIZE;

    uint8_t tile8[8 * 8];

    for (size_t i = 0; i < count; i++) {
        this->addTile();
        uint8_t* tile16 = this->_tiles.back().data();

        _Tileset__read8pxTile<BIT_DEPTH>(inData, tile8);
        _Tileset__insertTile8intoTile16(tile8, tile16, 0, 0);
        inData += SNES_8_DATA_SIZE;

        _Tileset__read8pxTile<BIT_DEPTH>(inData, tile8);
        _Tileset__insertTile8intoTile16(tile8, tile16, 8, 0);
        inData += SNES_8_DATA_SIZE;

        _Tileset__read8pxTile<BIT_DEPTH>(inData, tile8);
        _Tileset__insertTile8intoTile16(tile8, tile16, 0, 8);
        inData += SNES_8_DATA_SIZE;

        _Tileset__read8pxTile<BIT_DEPTH>(inData, tile8);
        _Tileset__insertTile8intoTile16(tile8, tile16, 8, 8);
        inData += SNES_8_DATA_SIZE;
    }
}
}
}
#endif
