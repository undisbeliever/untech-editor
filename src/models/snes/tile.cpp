#include "tile.h"
#include "tileset.h"
#include <cstring>

using namespace UnTech;
using namespace UnTech::Snes;

template <size_t BD>
Tile8px<BD>::Tile8px(const Tile<BD, 8>& t)
    : Tile<BD, 8>(t)
{
}

template <size_t BD>
Tile16px<BD>::Tile16px(const Tile<BD, 16>& t)
    : Tile<BD, 16>(t)
{
}

template <size_t BD, size_t TS>
void Tile<BD, TS>::draw(Image& image, const Palette<BD>& palette,
                        unsigned xOffset, unsigned yOffset,
                        const bool hFlip, const bool vFlip) const
{
    if (image.size().width < (xOffset + TILE_SIZE)
        || image.size().height < (yOffset + TILE_SIZE)) {

        return;
    }

    rgba* imgBits;
    const uint8_t* tilePos = rawData();

    for (unsigned y = 0; y < TILE_SIZE; y++) {
        if (!vFlip) {
            imgBits = image.scanline(yOffset + y);
        }
        else {
            imgBits = image.scanline(yOffset + TILE_SIZE - y - 1);
        }

        if (!hFlip) {
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
void Tile<BD, TS>::drawOpaque(Image& image, const Palette<BD>& palette,
                              unsigned xOffset, unsigned yOffset,
                              const bool hFlip, const bool vFlip) const
{
    if (image.size().width < (xOffset + TILE_SIZE)
        || image.size().height < (yOffset + TILE_SIZE)) {

        return;
    }

    rgba* imgBits;
    const uint8_t* tilePos = rawData();

    for (unsigned y = 0; y < TILE_SIZE; y++) {
        if (!vFlip) {
            imgBits = image.scanline(yOffset + y);
        }
        else {
            imgBits = image.scanline(yOffset + TILE_SIZE - y - 1);
        }

        if (!hFlip) {
            imgBits += xOffset;

            for (unsigned x = 0; x < TILE_SIZE; x++) {
                auto p = *tilePos & PIXEL_MASK;

                *imgBits = palette.color(p).rgb();
                imgBits++;
                tilePos++;
            }
        }
        else {
            imgBits += xOffset + TILE_SIZE - 1;

            for (unsigned x = 0; x < TILE_SIZE; x++) {
                auto p = *tilePos & PIXEL_MASK;

                *imgBits = palette.color(p).rgb();
                imgBits--;
                tilePos++;
            }
        }
    }
}

template <size_t BD, size_t TS>
inline typename Tile<BD, TS>::Tile_t Tile<BD, TS>::hFlip() const
{
    typename Tile<BD, TS>::Tile_t hFlip;
    uint8_t(*hData)[TS] = (uint8_t(*)[TS])hFlip.rawData();

    const auto pixelData = (uint8_t(*)[TS])this->rawData();

    for (unsigned y = 0; y < TS; y++) {
        for (unsigned x = 0; x < TS; x++) {
            hData[y][x] = pixelData[TILE_SIZE - y - 1][x];
        }
    }

    return hFlip;
}

template <size_t BD, size_t TS>
inline typename Tile<BD, TS>::Tile_t Tile<BD, TS>::vFlip() const
{
    typename Tile<BD, TS>::Tile_t vFlip;
    uint8_t(*vData)[TS] = (uint8_t(*)[TS])vFlip.rawData();

    const auto pixelData = (uint8_t(*)[TS])this->rawData();

    for (unsigned y = 0; y < TS; y++) {
        for (unsigned x = 0; x < TS; x++) {
            vData[y][x] = pixelData[y][TS - x - 1];
        }
    }

    return vFlip;
}

template <size_t BD, size_t TS>
inline typename Tile<BD, TS>::Tile_t Tile<BD, TS>::hvFlip() const
{
    typename Tile<BD, TS>::Tile_t hvFlip;
    uint8_t(*hvData)[TS] = (uint8_t(*)[TS])hvFlip.rawData();

    const auto pixelData = (uint8_t(*)[TS])this->rawData();

    for (unsigned y = 0; y < TS; y++) {
        for (unsigned x = 0; x < TS; x++) {
            hvData[y][x] = pixelData[TILE_SIZE - y - 1][TILE_SIZE - x - 1];
        }
    }

    return hvFlip;
}

template <size_t BD, size_t TS>
typename Tile<BD, TS>::Tile_t Tile<BD, TS>::flip(bool hFlip, bool vFlip) const
{
    if (hFlip == false) {
        if (vFlip == false) {
            return Tile<BD, TS>::Tile_t(*this);
        }
        else {
            return this->vFlip();
        }
    }
    else {
        if (vFlip == false) {
            return this->hFlip();
        }
        else {
            return this->hvFlip();
        }
    }
}

template <size_t BD>
inline std::array<Tile8px<BD>, 4> Tile16px<BD>::splitIntoSmall() const
{
    std::array<Tile8px<BD>, 4> ret;

    const uint8_t* tile16 = this->rawData();

    auto transform = [](const uint8_t tile16[256], Tile8px<BD>& tile8,
                        unsigned xPos, unsigned yPos) {
        const uint8_t* t16 = tile16 + (yPos * 16 + xPos);
        uint8_t* t8 = tile8.rawData();

        for (unsigned y = 0; y < 8; y++) {
            memcpy(t8, t16, 8);
            t16 += 16;
            t8 += 8;
        }
    };

    transform(tile16, ret[0], 0, 0);
    transform(tile16, ret[1], 8, 0);
    transform(tile16, ret[2], 0, 8);
    transform(tile16, ret[3], 8, 8);

    return ret;
}

template <size_t BD>
inline void Tile16px<BD>::combineIntoLarge(const std::array<Tile8px<BD>, 4>& tiles)
{
    uint8_t* tile16 = this->rawData();

    auto transform = [](const Tile8px<BD>& tile8, uint8_t tile16[256],
                        unsigned xPos, unsigned yPos) {

        const uint8_t* t8 = tile8.rawData();
        uint8_t* t16 = tile16 + (yPos * 16 + xPos);

        for (unsigned y = 0; y < 8; y++) {
            memcpy(t16, t8, 8);
            t8 += 8;
            t16 += 16;
        }
    };

    transform(tiles[0], tile16, 0, 0);
    transform(tiles[1], tile16, 8, 0);
    transform(tiles[2], tile16, 0, 8);
    transform(tiles[3], tile16, 8, 8);
}

template <size_t BD>
inline void Tile8px<BD>::readSnesData(const uint8_t data[SNES_DATA_SIZE])
{
    const uint8_t* dataPos = data;
    uint8_t* tile = this->rawData();

    memset(tile, 0, TILE_ARRAY_SIZE);

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

template <size_t BD>
inline void Tile16px<BD>::readSnesData(const uint8_t data[SNES_DATA_SIZE])
{
    const uint8_t* inPos = data;
    std::array<Tile8px<BD>, 4> smallTiles;

    for (auto& tile8 : smallTiles) {
        tile8.readSnesData(inPos);
        inPos += Tile8px<BD>::SNES_DATA_SIZE;
    }

    combineIntoLarge(smallTiles);
}

template <size_t BD>
inline void Tile8px<BD>::writeSnesData(uint8_t out[SNES_DATA_SIZE]) const
{
    const uint8_t* tile = this->rawData();

    unsigned pos = 0;
    memset(out, 0, SNES_DATA_SIZE);

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
                out[pos++] = byte;
            }
        }
    }
}

template <size_t BD>
inline void Tile16px<BD>::writeSnesData(uint8_t out[SNES_DATA_SIZE]) const
{
    uint8_t* outPos = out;

    auto smallTiles = splitIntoSmall();

    for (auto& tile : smallTiles) {
        tile.writeSnesData(outPos);
        outPos += Tile8px<BD>::SNES_DATA_SIZE;
    }
}

// Hash Functions
// --------------
namespace std {

template <size_t BD>
size_t hash<Tile8px<BD>>::operator()(const Tile8px<BD>& tile) const
{
    const uint8_t* data = tile.rawData();

    size_t seed = 0;
    for (unsigned i = 0; i < tile.TILE_ARRAY_SIZE; i++) {
        // Numbers from boost
        seed ^= data[i] + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    return seed;
}

template <size_t BD>
size_t hash<Tile16px<BD>>::operator()(const Tile16px<BD>& tile) const
{
    const uint8_t* data = tile.rawData();

    size_t seed = 0;
    for (unsigned i = 0; i < tile.TILE_ARRAY_SIZE; i++) {
        // Numbers from boost
        seed ^= data[i] + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    return seed;
}
}

template struct std::hash<Tile8px<2>>;
template struct std::hash<Tile8px<4>>;
template struct std::hash<Tile8px<8>>;
template struct std::hash<Tile16px<4>>;

template class Snes::Tile<2, 8>;
template class Snes::Tile<4, 8>;
template class Snes::Tile<8, 8>;
template class Snes::Tile<4, 16>;

template class Snes::Tile8px<2>;
template class Snes::Tile8px<4>;
template class Snes::Tile8px<8>;
template class Snes::Tile16px<4>;
