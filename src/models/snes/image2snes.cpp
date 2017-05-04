#include "image2snes.h"
#include "tilesetinserter.h"

#include <algorithm>
#include <array>
#include <vector>

namespace UnTech {
namespace Snes {

class Image2SnesConverter {
    constexpr static unsigned TILE_SIZE = 8;
    constexpr static unsigned TILE_DATA_SIZE = TILE_SIZE * TILE_SIZE;

    constexpr static unsigned MAX_N_PALETTES = 8;
    constexpr static unsigned MAX_N_TILES = 1024;

private:
    const unsigned colorsPerPalette;
    const unsigned tileOffset;
    const unsigned maxTiles;
    const unsigned paletteOffset;
    const unsigned maxPalettes;
    const bool defaultOrder;

    // size of the image in tile maps.
    unsigned mapWidth;
    unsigned mapHeight;

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
        , mapWidth(0)
        , mapHeight(0)
        , palette()
        , tiles()
        , tilePaletteId()
    {
        if (maxTiles > MAX_N_TILES) {
            throw std::invalid_argument("maxTiles is too large (expected <= "
                                        + std::to_string(MAX_N_TILES) + ")");
        }
        if (tileOffset + maxTiles > MAX_N_TILES) {
            throw std::invalid_argument("tileOffset is out of bounds "
                                        "(tileOffset + maxTiles must be <= "
                                        + std::to_string(MAX_N_TILES) + ")");
        }
        if (maxPalettes > MAX_N_PALETTES) {
            throw std::invalid_argument("maxPalettes is too large (expected <= "
                                        + std::to_string(MAX_N_PALETTES) + ")");
        }
        if (paletteOffset + maxPalettes > MAX_N_PALETTES) {
            throw std::invalid_argument("paletteOffset is out of bounds "
                                        "(paletteOffset + maxPalettes must be <= "
                                        + std::to_string(MAX_N_PALETTES) + ")");
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
    Tilemap buildTilemapAndTileset(BaseTileset<8>& tileset)
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
        for (unsigned mapId = 0; mapId < tilemap.nMaps(); mapId++) {
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
            throw std::runtime_error("Too many tiles in the image ("
                                     + std::to_string(tileset.size()) + " tiles required, "
                                     + "maxTiles is " + std::to_string(maxTiles) + ")");
        }

        return tilemap;
    }

    // `process()` MUST be called before calling `savePalette`
    std::vector<SnesColor> buildSnesColorPalette()
    {
        std::vector<SnesColor> out(palette.size());

        for (unsigned i = 0; i < palette.size(); i++) {
            out[i].setData(palette[i]);
        }

        return out;
    }

private:
    void buildTileData(const IndexedImage& image)
    {
        // expands to cover an entire tilemap
        // tiles are rearranged to match the SNES tilemap order.

        if (image.size().width % TILE_SIZE != 0 || image.size().height % TILE_SIZE != 0) {
            throw std::runtime_error("Image size is not divisible by " + std::to_string(TILE_SIZE));
        }

        constexpr unsigned MAP_SIZE = Tilemap::MAP_SIZE;

        unsigned tw = (image.size().width + TILE_SIZE - 1) / TILE_SIZE;
        unsigned th = (image.size().height + TILE_SIZE - 1) / TILE_SIZE;

        mapWidth = (tw - 1) / MAP_SIZE + 1;
        mapHeight = (th - 1) / MAP_SIZE + 1;

        tiles.clear();
        tiles.reserve(mapWidth * mapHeight * MAP_SIZE * MAP_SIZE);

        for (unsigned mapY = 0; mapY < mapHeight; mapY++) {
            for (unsigned mapX = 0; mapX < mapWidth; mapX++) {
                for (unsigned ty = 0; ty < MAP_SIZE; ty++) {
                    for (unsigned tx = 0; tx < MAP_SIZE; tx++) {
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
        std::array<uint8_t, TILE_DATA_SIZE> tile;
        uint8_t* tData = tile.data();

        for (unsigned py = 0; py < TILE_SIZE; py++) {
            const uint8_t* imgData = image.scanline(y + py) + x;

            for (unsigned px = 0; px < TILE_SIZE; px++) {
                *tData++ = *imgData++;
            }
        }

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

        for (unsigned i = 0; i < sourcePalette.size(); i++) {
            palette[i] = SnesColor(sourcePalette[i]).data();
        }
    }

    void removeDuplicateColors()
    {
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
            for (unsigned i = 0; i < nColors; i++) {
                if (colors[i] == pixel) {
                    return true;
                }
            }
            return false;
        }

        void addColor(unsigned pixel)
        {
            if (nColors < MAX_PALETTE_COLORS) {
                colors[nColors] = pixel;
                nColors++;
            }
        }

        unsigned countMatchingColors(const TileColors& cmp) const
        {
            unsigned nMatches = 0;

            for (unsigned i = 0; i < this->nColors; i++) {
                for (unsigned j = 0; j < cmp.nColors; j++) {
                    if (this->colors[i] == cmp.colors[j]) {
                        nMatches++;
                        break;
                    }
                }
            }

            return nMatches;
        }

        void addMissingColors(const TileColors& toAdd)
        {
            for (unsigned i = 0; i < toAdd.nColors; i++) {
                unsigned c = toAdd.colors[i];

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

        for (unsigned tileId = 0; tileId < tiles.size(); tileId++) {
            const auto& tile = tiles[tileId];
            TileColors& tp = tileColors[tileId];

            for (const uint8_t pixel : tile) {
                if (pixel != 0 && tp.containsColor(pixel) == false) {
                    if (tp.nColors >= colorsPerPalette) {
                        throw std::runtime_error("Tile contains too many colors");
                    }
                    else {
                        tp.addColor(pixel);
                    }
                }
            }
        }

        return tileColors;
    }

    inline std::vector<TileColors> rearrangePalette_buildNewPalette(const std::vector<TileColors>& colorsPerTile)
    {
        // Sort the tile indexes by palette size (largest to smallest).
        // As nColors is limited, this is faster than std::sort.
        // ::KUDOS pcx2snes.c by Neviksti::

        std::vector<unsigned> processOrder;
        processOrder.reserve(tiles.size());
        for (unsigned c = 0; c < TileColors::MAX_PALETTE_COLORS + 1; c++) {
            unsigned colorToTest = TileColors::MAX_PALETTE_COLORS - c;

            for (unsigned i = 0; i < tiles.size(); i++) {
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

            for (unsigned i = 0; i < newPalette.size(); i++) {
                const auto& toTest = newPalette[i];

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
            throw std::runtime_error("Could not rearrange the palette ("
                                     + std::to_string(newPalette.size()) + " palettes required, "
                                     + "maxPalettes is " + std::to_string(maxPalettes) + ")");
        }

        return newPalette;
    }

    inline void rearrangePalette_rewritePaletteData(const std::vector<TileColors>& newPalette)
    {
        std::vector<uint16_t> oldPalette = palette;
        palette.assign(newPalette.size() * colorsPerPalette, 0);

        for (unsigned p = 0; p < newPalette.size(); p++) {
            auto& pal = newPalette[p];
            unsigned startingColor = p * colorsPerPalette;

            palette[startingColor] = oldPalette[0];

            for (unsigned c = 0; c < pal.nColors; c++) {
                palette[startingColor + c + 1] = oldPalette.at(pal.colors[c]);
            }
        }
    }

    inline void rearrangePalette_rewriteTileData(const std::vector<TileColors>& newPalette)
    {
        // mapping of tile data to palette color.
        std::vector<std::array<uint8_t, 256>> paletteMap(newPalette.size());

        for (unsigned i = 0; i < newPalette.size(); i++) {
            auto& map = paletteMap[i];
            auto& pal = newPalette[i];

            for (unsigned c = 0; c < pal.nColors; c++) {
                map[pal.colors[c]] = c + 1;
            }
        }

        for (unsigned i = 0; i < tiles.size(); i++) {
            auto& tile = tiles[i];
            unsigned pal = tilePaletteId[i];
            auto& map = paletteMap.at(pal);

            for (auto& c : tile) {
                c = map[c];
            }
        }
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

template <size_t BIT_DEPTH>
void Image2Snes<BIT_DEPTH>::process(const IndexedImage& image)
{
    if (BIT_DEPTH > 4) {
        _maxPalettes = 1;
        _paletteOffset = 0;
    }

    Image2SnesConverter converter(COLORS_PER_PALETTE,
                                  _tileOffset, _maxTiles,
                                  _paletteOffset, _maxPalettes,
                                  _order);

    converter.process(image);

    _tilemap = converter.buildTilemapAndTileset(_tileset);
    _palette = converter.buildSnesColorPalette();
}

template <size_t BIT_DEPTH>
std::vector<uint8_t> Image2Snes<BIT_DEPTH>::paletteSnesData() const
{
    std::vector<uint8_t> data(_palette.size() * 2);
    auto* ptr = data.data();

    for (const auto& c : _palette) {
        *ptr++ = c.data() & 0xFF;
        *ptr++ = c.data() >> 8;
    }

    return data;
}

template class Image2Snes<1>;
template class Image2Snes<2>;
template class Image2Snes<3>;
template class Image2Snes<4>;
template class Image2Snes<8>;
}
}