/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "image2snes.h"
#include "bit-depth.h"
#include "tilesetinserter.h"
#include "models/common/exceptions.h"
#include "models/common/iterators.h"
#include "models/common/stringbuilder.h"
#include "models/snes/convert-snescolor.h"
#include <algorithm>
#include <array>
#include <vector>

namespace UnTech::Snes {

class Image2SnesConverter {
    constexpr static unsigned TILE_SIZE = 8;
    constexpr static unsigned TILE_DATA_SIZE = TILE_SIZE * TILE_SIZE;

    constexpr static unsigned MAX_N_PALETTES = 8;
    constexpr static unsigned MAX_N_TILES = 1024;

private:
    unsigned colorsPerPalette;
    unsigned tileOffset;
    unsigned maxTiles;
    unsigned paletteOffset;
    unsigned maxPalettes;
    bool defaultOrder;

    // size of the image in tile maps.
    unsigned mapWidth{ 0 };
    unsigned mapHeight{ 0 };

    std::vector<uint16_t> palette;

    // tile data, tile order matches that of the SNES's Tilemap
    std::vector<std::array<uint8_t, TILE_DATA_SIZE>> tiles;

    // Mapping of tiles -> palette
    std::vector<unsigned> tilePaletteId;

public:
    Image2SnesConverter(unsigned colorsPerPalette,
                        unsigned tileOffset, unsigned maxTiles,
                        unsigned paletteOffset, unsigned maxPalettes,
                        bool defaultOrder)
        : colorsPerPalette(colorsPerPalette)
        , tileOffset(tileOffset)
        , maxTiles(maxTiles)
        , paletteOffset(paletteOffset)
        , maxPalettes(maxPalettes)
        , defaultOrder(defaultOrder)
        , palette()
        , tiles()
        , tilePaletteId()
    {
        if (maxTiles > MAX_N_TILES) {
            throw invalid_argument(u8"maxTiles is too large (expected <= ", MAX_N_TILES, u8")");
        }
        if (tileOffset + maxTiles > MAX_N_TILES) {
            throw invalid_argument(u8"tileOffset is out of bounds (tileOffset + maxTiles must be <= ", MAX_N_TILES, u8")");
        }
        if (maxPalettes > MAX_N_PALETTES) {
            throw invalid_argument(u8"maxPalettes is too large (expected <= ", MAX_N_PALETTES, u8")");
        }
        if (paletteOffset + maxPalettes > MAX_N_PALETTES) {
            throw invalid_argument(u8"paletteOffset is out of bounds (paletteOffset + maxPalettes must be <= ", MAX_N_PALETTES, u8")");
        }
    }

    void process(const IndexedImage& image)
    {
        buildTileData(image);
        buildPalette(image);
        removeDuplicateColors();
        rearrangePalette();
    }

    // `process()` MUST be called before calling `buildTilesetAndTilemap`
    Tilemap buildTilemapAndTileset(std::vector<Tile8px>& tileset)
    {
        assert(mapWidth >= 1);
        assert(mapHeight >= 1);
        assert(tiles.size() == mapWidth * mapHeight * Tilemap::MAP_SIZE * Tilemap::MAP_SIZE);
        assert(tilePaletteId.size() == tiles.size());

        Tilemap tilemap = Tilemap(mapWidth, mapHeight);

        TilesetInserter8px tilesetInserter(tileset);
        Tile8px tile = {};

        if (containsTransparentTile()) {
            // make the transparent tile the first one
            tilesetInserter.getOrInsert(tile);
        }

        // tiles/tilePaletteId are in the same order as the tilemap
        unsigned tileId = 0;
        for (const auto mapId : range(tilemap.nMaps())) {
            for (auto& cell : tilemap.map(mapId)) {
                tile.setData(tiles[tileId]);
                auto to = tilesetInserter.getOrInsert(tile);

                cell.setCharacter(to.tileId + tileOffset);
                cell.setPalette(tilePaletteId[tileId] + paletteOffset);
                cell.setOrder(defaultOrder);
                cell.setHFlip(to.hFlip);
                cell.setVFlip(to.vFlip);

                tileId++;
            }
        }

        if (tileset.size() > maxTiles) {
            throw runtime_error(u8"Too many tiles in the image (", tileset.size(), u8" tiles required, maxTiles is ", maxTiles, u8")");
        }

        return tilemap;
    }

    // `process()` MUST be called before calling `savePalette`
    std::vector<SnesColor> buildSnesColorPalette()
    {
        std::vector<SnesColor> out(palette.size());
        auto outIt = out.begin();

        for (const auto& c : palette) {
            (outIt++)->setData(c);
        }
        assert(outIt == out.end());

        return out;
    }

private:
    void buildTileData(const IndexedImage& image)
    {
        // expands to cover an entire tilemap
        // tiles are rearranged to match the SNES tilemap order.

        if (image.size().width % TILE_SIZE != 0 || image.size().height % TILE_SIZE != 0) {
            throw runtime_error(u8"Image size is not divisible by ", TILE_SIZE);
        }

        constexpr unsigned MAP_SIZE = Tilemap::MAP_SIZE;

        unsigned tw = (image.size().width + TILE_SIZE - 1) / TILE_SIZE;
        unsigned th = (image.size().height + TILE_SIZE - 1) / TILE_SIZE;

        mapWidth = (tw - 1) / MAP_SIZE + 1;
        mapHeight = (th - 1) / MAP_SIZE + 1;

        tiles.clear();
        tiles.reserve(mapWidth * mapHeight * MAP_SIZE * MAP_SIZE);

        for (const auto mapY : range(mapHeight)) {
            for (const auto mapX : range(mapWidth)) {
                for (const auto ty : range(MAP_SIZE)) {
                    for (const auto tx : range(MAP_SIZE)) {
                        unsigned x = (mapX * MAP_SIZE + tx) * TILE_SIZE;
                        unsigned y = (mapY * MAP_SIZE + ty) * TILE_SIZE;

                        if (x + TILE_SIZE <= image.size().width && y + TILE_SIZE <= image.size().height) {
                            loadTile(image, x, y);
                        }
                        else {
                            loadBlankTile();
                        }
                    }
                }
            }
        }
    }

    inline void loadTile(const IndexedImage& image, unsigned x, unsigned y)
    {
        std::array<uint8_t, TILE_DATA_SIZE> tile{};
        auto tData = tile.begin();

        assert(x + TILE_SIZE < image.size().width && y + TILE_SIZE < image.size().height);

        for (const auto py : range(TILE_SIZE)) {
            const auto imgBits = image.scanline(y + py).subspan(x, TILE_SIZE);

            for (const auto& c : imgBits) {
                *tData++ = c;
            }
        }
        assert(tData = tile.end());

        tiles.emplace_back(tile);
    }

    inline void loadBlankTile()
    {
        const std::array<uint8_t, TILE_DATA_SIZE> BLANK_TILE = {};
        tiles.emplace_back(BLANK_TILE);
    }

    void buildPalette(const IndexedImage& image)
    {
        const auto& sourcePalette = image.palette();

        palette.resize(image.palette().size());
        auto palIt = palette.begin();

        for (const auto& sp : sourcePalette) {
            *palIt++ = Snes::toSnesColor(sp).data();
        }
        assert(palIt == palette.end());
    }

    void removeDuplicateColors()
    {
        // Must use an old-style for loop, palette is resized inside this loop.
        for (unsigned i = 0; i < palette.size(); i++) {
            unsigned j = i + 1;
            while (j < palette.size()) {
                if (palette[i] == palette[j]) {
                    palette.erase(palette.begin() + j);

                    for (auto& tile : tiles) {
                        for (auto& p : tile) {
                            if (p == j) {
                                p = i;
                            }
                            else if (p > j) {
                                p = p - 1;
                            }
                        }
                    }
                }
                else {
                    j++;
                }
            }
        }
    }

    struct TileColors {
        static_assert(sizeof(unsigned) >= sizeof(uint16_t), "Bad optimisation");

        constexpr static unsigned MAX_PALETTE_COLORS = 15;

        unsigned nColors = 0;
        std::array<unsigned, MAX_PALETTE_COLORS> colors = {};

        TileColors() = default;

        bool containsColor(unsigned pixel)
        {
            for (const auto& c : colors) {
                if (c == pixel) {
                    return true;
                }
            }
            return false;
        }

        void addColor(unsigned pixel)
        {
            if (nColors < MAX_PALETTE_COLORS) {
                colors.at(nColors) = pixel;
                nColors++;
            }
        }

        [[nodiscard]] unsigned countMatchingColors(const TileColors& cmp) const
        {
            unsigned nMatches = 0;

            for (const auto i : range(this->nColors)) {
                for (const auto j : range(cmp.nColors)) {
                    if (this->colors.at(i) == cmp.colors.at(j)) {
                        nMatches++;
                        break;
                    }
                }
            }

            return nMatches;
        }

        void addMissingColors(const TileColors& toAdd)
        {
            for (const auto i : range(toAdd.nColors)) {
                const unsigned c = toAdd.colors.at(i);

                if (this->containsColor(c) == false) {
                    this->addColor(c);
                }
            }
        }
    };

    void rearrangePalette()
    {
        if (palette.size() <= colorsPerPalette) {
            // no need to resize palette
            tilePaletteId.assign(tiles.size(), 0);
            return;
        }

        std::vector<TileColors> tileColors = rearrangePalette_colorsPerTile();
        std::vector<TileColors> newPalette = rearrangePalette_buildNewPalette(tileColors);

        rearrangePalette_rewritePaletteData(newPalette);
        rearrangePalette_rewriteTileData(newPalette);
    }

    inline std::vector<TileColors> rearrangePalette_colorsPerTile()
    {
        std::vector<TileColors> tileColors(tiles.size());
        auto it = tileColors.begin();

        for (const auto& tile : tiles) {
            TileColors& tp = *it++;

            for (const uint8_t pixel : tile) {
                if (pixel != 0 && tp.containsColor(pixel) == false) {
                    if (tp.nColors >= colorsPerPalette) {
                        throw runtime_error(u8"Tile contains too many colors");
                    }
                    else {
                        tp.addColor(pixel);
                    }
                }
            }
        }
        assert(it == tileColors.end());

        return tileColors;
    }

    inline std::vector<TileColors> rearrangePalette_buildNewPalette(const std::vector<TileColors>& colorsPerTile)
    {
        // Sort the tile indexes by palette size (largest to smallest).
        // As nColors is limited, this is faster than std::sort.
        // ::KUDOS pcx2snes.c by Neviksti::

        std::vector<unsigned> processOrder;
        processOrder.reserve(tiles.size());
        for (const auto colorToTest : reverse_range(TileColors::MAX_PALETTE_COLORS)) {
            for (const auto [i, cpt] : enumerate(colorsPerTile)) {
                if (colorsPerTile[i].nColors == colorToTest) {
                    processOrder.push_back(i);
                }
            }
        }
        assert(processOrder.size() == colorsPerTile.size());

        // Build the final palette, processing the tiles with the most colors first.

        std::vector<TileColors> newPalette;
        newPalette.reserve(maxPalettes);

        tilePaletteId.assign(tiles.size(), 0);

        for (unsigned tileId : processOrder) {
            const auto& tileColors = colorsPerTile[tileId];

            if (tileColors.nColors == 0) {
                continue;
            }

            int bestMatch = -1;
            unsigned bestIndex = 0;

            for (auto [i, toTest] : const_enumerate(newPalette)) {
                int nMatching = tileColors.countMatchingColors(toTest);
                int nMissing = tileColors.nColors - nMatching;

                if (nMatching > bestMatch
                    && toTest.nColors + nMissing < colorsPerPalette) {

                    bestMatch = nMatching;
                    bestIndex = i;
                }
            }

            if (bestMatch == (int)tileColors.nColors) {
                // Palette already exists
                tilePaletteId[tileId] = bestIndex;
            }
            else if (bestMatch >= 0) {
                // Append missing colors to existing palette
                newPalette[bestIndex].addMissingColors(tileColors);

                tilePaletteId[tileId] = bestIndex;
            }
            else {
                newPalette.push_back(tileColors);
                tilePaletteId[tileId] = newPalette.size() - 1;
            }
        }

        if (newPalette.size() > maxPalettes) {
            throw runtime_error(u8"Could not rearrange the palette (", newPalette.size(), u8" palettes required, maxPalettes is ", maxPalettes, u8")");
        }

        return newPalette;
    }

    inline void rearrangePalette_rewritePaletteData(const std::vector<TileColors>& newPalette)
    {
        std::vector<uint16_t> oldPalette = palette;
        palette.assign(newPalette.size() * colorsPerPalette, 0);

        for (auto [p, pal] : const_enumerate(newPalette)) {
            unsigned startingColor = p * colorsPerPalette;

            palette[startingColor] = oldPalette[0];

            for (const auto c : range(pal.nColors)) {
                palette.at(startingColor + c + 1) = oldPalette.at(pal.colors.at(c));
            }
        }
    }

    inline void rearrangePalette_rewriteTileData(const std::vector<TileColors>& newPalette)
    {
        // mapping of tile data to palette color.
        std::vector<std::array<uint8_t, 256>> paletteMap(newPalette.size());
        auto pIt = paletteMap.begin();

        for (const auto& pal : newPalette) {
            auto& map = *pIt++;

            for (const auto c : range(pal.nColors)) {
                map.at(pal.colors.at(c)) = c + 1;
            }
        }
        assert(pIt == paletteMap.end());

        auto tIt = tiles.begin();

        for (const auto& pal : tilePaletteId) {
            const auto& map = paletteMap.at(pal);
            auto& tile = *tIt++;

            for (auto& c : tile) {
                c = map[c];
            }
        }
        assert(tIt == tiles.end());
    }

    inline bool containsTransparentTile()
    {
        const static std::array<uint8_t, TILE_DATA_SIZE> BLANK_TILE = {};

        // last tile is most likely transparent.

        auto it = std::find(tiles.rbegin(), tiles.rend(), BLANK_TILE);
        return it != tiles.rend();
    }
};

/*
 * PUBLIC API
 * ==========
 */

Image2Snes::Image2Snes(BitDepthSpecial bd)
    : _bitDepth(bd)
    , _palette()
    , _tilemap()
{
}

void Image2Snes::process(const IndexedImage& image)
{
    if (_bitDepth > BitDepthSpecial::BD_4BPP) {
        _maxPalettes = 1;
        _paletteOffset = 0;
    }

    Image2SnesConverter converter(colorsForBitDepth(_bitDepth),
                                  _tileOffset, _maxTiles,
                                  _paletteOffset, _maxPalettes,
                                  _order);

    converter.process(image);

    _tilemap = converter.buildTilemapAndTileset(_tileset);
    _palette = converter.buildSnesColorPalette();
}

std::vector<uint8_t> Image2Snes::paletteSnesData() const
{
    std::vector<uint8_t> out(_palette.size() * 2);
    auto outIt = out.begin();

    for (const auto& c : _palette) {
        *outIt++ = c.data() & 0xFF;
        *outIt++ = c.data() >> 8;
    }
    assert(outIt == out.end());

    return out;
}

}
