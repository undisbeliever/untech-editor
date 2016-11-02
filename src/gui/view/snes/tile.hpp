#pragma once

#include "models/snes/tile.h"
#include <wx/rawbmp.h>

namespace UnTech {
namespace View {
namespace Snes {

template <bool showTransparent, size_t BD, class PI>
inline void __DrawTilePixel(PI& pit, const UnTech::Snes::Palette<BD>& palette, const size_t c);

template <bool showTransparent, size_t BD>
inline void __DrawTilePixel(wxNativePixelData::Iterator& pIt,
                            const UnTech::Snes::Palette<BD>& palette, const size_t c)
{
    if (showTransparent || c != 0) {
        auto rgb = palette.color(c).rgb();

        pIt.Red() = rgb.red;
        pIt.Green() = rgb.green;
        pIt.Blue() = rgb.blue;
    }
}

template <bool showTransparent, size_t BD>
inline void __DrawTilePixel(wxAlphaPixelData::Iterator& pIt,
                            const UnTech::Snes::Palette<BD>& palette, const size_t c)
{
    if (showTransparent || c != 0) {
        auto rgb = palette.color(c).rgb();

        pIt.Red() = rgb.red;
        pIt.Green() = rgb.green;
        pIt.Blue() = rgb.blue;
        pIt.Alpha() = 0xff;
    }
}

template <size_t BD, size_t TS, class PF, bool showTransparent>
void __DrawTile(PF& pixelData,
                const UnTech::Snes::Tile<BD, TS>& tile,
                const UnTech::Snes::Palette<BD>& palette,
                const unsigned xOffset, const unsigned yOffset,
                const bool hFlip, const bool vFlip)
{
    const unsigned TILE_SIZE = tile.TILE_SIZE;
    const unsigned PIXEL_MASK = tile.PIXEL_MASK;

    if (pixelData.GetWidth() < int(xOffset + TILE_SIZE)
        || pixelData.GetHeight() < int(yOffset + TILE_SIZE)) {

        return;
    }

    typename PF::Iterator imgBits(pixelData);
    typename PF::Iterator rowIt(pixelData);
    rowIt.Offset(pixelData, xOffset, yOffset);

    if (!vFlip) {
        if (!hFlip) {
            // no Flip
            const uint8_t* tilePos = tile.rawData();

            for (unsigned y = 0; y < TILE_SIZE; y++) {
                imgBits = rowIt;
                for (unsigned x = 0; x < TILE_SIZE; x++) {
                    __DrawTilePixel<showTransparent>(imgBits, palette, *tilePos & PIXEL_MASK);

                    ++imgBits;
                    ++tilePos;
                }
                rowIt.OffsetY(pixelData, 1);
            }
        }
        else {
            // hFlip
            const uint8_t* rowData = tile.rawData() + TILE_SIZE - 1;

            for (unsigned y = 0; y < TILE_SIZE; y++) {
                const uint8_t* tilePos = rowData;

                imgBits = rowIt;
                for (unsigned x = 0; x < TILE_SIZE; x++) {
                    __DrawTilePixel<showTransparent>(imgBits, palette, *tilePos & PIXEL_MASK);

                    ++imgBits;
                    --tilePos;
                }
                rowData += TILE_SIZE;
                rowIt.OffsetY(pixelData, 1);
            }
        }
    }
    else {
        if (!hFlip) {
            // vFlip
            const uint8_t* rowData = tile.rawData() + TILE_SIZE * (TILE_SIZE - 1);

            for (unsigned y = 0; y < TILE_SIZE; y++) {
                const uint8_t* tilePos = rowData;

                imgBits = rowIt;
                for (unsigned x = 0; x < TILE_SIZE; x++) {
                    __DrawTilePixel<showTransparent>(imgBits, palette, *tilePos & PIXEL_MASK);

                    ++imgBits;
                    ++tilePos;
                }
                rowData -= TILE_SIZE;
                rowIt.OffsetY(pixelData, 1);
            }
        }
        else {
            // hvFlip
            const uint8_t* tilePos = tile.rawData() + (TILE_SIZE * TILE_SIZE) - 1;

            for (unsigned y = 0; y < TILE_SIZE; y++) {
                imgBits = rowIt;
                for (unsigned x = 0; x < TILE_SIZE; x++) {
                    __DrawTilePixel<showTransparent>(imgBits, palette, *tilePos & PIXEL_MASK);

                    ++imgBits;
                    --tilePos;
                }
                rowIt.OffsetY(pixelData, 1);
            }
        }
    }
}

template <size_t BD, size_t TS, class PF>
void DrawTileOpaque(PF& pixelData,
                    const UnTech::Snes::Tile<BD, TS>& tile,
                    const UnTech::Snes::Palette<BD>& palette,
                    const unsigned xOffset, const unsigned yOffset,
                    const bool hFlip, const bool vFlip)
{
    __DrawTile<BD, TS, PF, true>(pixelData, tile, palette, xOffset, yOffset, hFlip, vFlip);
}

template <size_t BD, size_t TS, class PF>
void DrawTileTransparent(PF& pixelData,
                         const UnTech::Snes::Tile<BD, TS>& tile,
                         const UnTech::Snes::Palette<BD>& palette,
                         const unsigned xOffset, const unsigned yOffset,
                         const bool hFlip, const bool vFlip)
{
    __DrawTile<BD, TS, PF, false>(pixelData, tile, palette, xOffset, yOffset, hFlip, vFlip);
}
}
}
}
