/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "invalid-image-error.h"
#include "models/common/image.h"
#include "models/common/iterators.h"
#include "models/snes/convert-snescolor.h"
#include "models/snes/tile.h"
#include <vector>

namespace UnTech::Resources {

struct TileAndPalette {
    Snes::Tile8px tile;
    unsigned palette;
};

inline bool extractTile8px(Snes::Tile8px& tile,
                           const Image& image, const unsigned x, const unsigned y,
                           const std::vector<Snes::SnesColor>::const_iterator pBegin,
                           const std::vector<Snes::SnesColor>::const_iterator pEnd)
{
    const static unsigned TS = tile.TILE_SIZE;

    assert(x + TS <= image.size().width && x + TS <= image.size().height);

    for (const auto ty : range(TS)) {
        auto imgBits = image.scanline(y + ty).subspan(x, TS);
        for (const auto tx : range(TS)) {
            const Snes::SnesColor c = Snes::toSnesColor(imgBits[tx]);

            auto pIt = std::find(pBegin, pEnd, c);
            if (pIt == pEnd) {
                // color not found in palette
                return false;
            }

            tile.setPixel(tx, ty, std::distance(pBegin, pIt));
        }
    }

    return true;
}

inline bool extractTileAndPalette(TileAndPalette& ft, const Image& image, const unsigned x, const unsigned y,
                                  const std::vector<Snes::SnesColor>& palette, const unsigned colorsPerPalette,
                                  const unsigned firstPalette, const unsigned nPalettes)
{
    const unsigned firstColor = firstPalette * colorsPerPalette;
    const unsigned lastColor = std::min<size_t>(firstColor + nPalettes * colorsPerPalette, palette.size());

    for (size_t pIndex = firstColor, pId = firstPalette; pIndex < lastColor; pIndex += colorsPerPalette, pId++) {
        auto pStart = palette.begin() + pIndex;
        auto pEnd = pIndex + colorsPerPalette < lastColor ? pStart + colorsPerPalette
                                                          : palette.begin() + lastColor;

        bool s = extractTile8px(ft.tile, image, x, y, pStart, pEnd);

        if (s) {
            ft.palette = pId;
            return true;
        }
    }

    return false;
}

inline std::vector<TileAndPalette> tilesFromImage(const Image& image, const unsigned bitDepth,
                                                  const std::vector<Snes::SnesColor>& palette,
                                                  const unsigned firstPalette, const unsigned nPalettes,
                                                  std::vector<InvalidImageTile>& err)
{
    const static unsigned TS = decltype(TileAndPalette::tile)::TILE_SIZE;

    const unsigned colorsPerPalette = 1 << bitDepth;

    const usize iSize = image.size();

    unsigned tw = iSize.width / TS;
    unsigned th = iSize.height / TS;
    std::vector<TileAndPalette> tiles(tw * th);
    auto tileIt = tiles.begin();

    for (unsigned y = 0; y < iSize.height; y += TS) {
        for (unsigned x = 0; x < iSize.width; x += TS) {
            bool s = extractTileAndPalette(*tileIt, image, x, y, palette, colorsPerPalette, firstPalette, nPalettes);
            if (!s) {
                err.emplace_back(TS, x, y, InvalidTileReason::NO_PALETTE_FOUND);
            }
            tileIt++;
        }
    }
    assert(tileIt == tiles.end());

    return tiles;
}

}
