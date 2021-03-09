/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "shaders.h"
#include "models/common/iterators.h"
#include "models/metatiles/interactive-tiles.h"
#include "models/metatiles/metatile-tileset.h"
#include "models/project/project-data.h"
#include "models/project/project.h"
#include <cassert>

#if defined(IMGUI_IMPL_SDL_OPENGL)
#include "opengl/shaders_opengl3.hpp"
#endif

namespace UnTech::Gui::Shaders {

static constexpr unsigned METATILE_SIZE_PX = MetaTiles::METATILE_SIZE_PX;
static constexpr unsigned TILESET_WIDTH = MetaTiles::TILESET_WIDTH;
static constexpr unsigned TILESET_HEIGHT = MetaTiles::TILESET_HEIGHT;

static Image drawPaletteImage(const Resources::PaletteData& palette)
{
    Image image(UINT8_MAX, palette.paletteFrames.size());
    image.fill(rgba(255, 0, 255, 255));

    for (auto [palIndex, palFrame] : enumerate(palette.paletteFrames)) {
        const unsigned nColors = std::min<unsigned>(palFrame.size(), UINT8_MAX);

        rgba* imgBits = image.scanline(palIndex);
        for (unsigned c = 0; c < nColors; c++) {
            imgBits[c] = palFrame.at(c).rgb();
        }
        assert(imgBits <= image.data() + image.dataSize());
    }

    return image;
}

static void drawAnimatedTileset(grid<uint8_t>& image, const Resources::AnimatedTilesetData& animatedTileset, unsigned frameIndex)
{
    constexpr unsigned TILE_SIZE = Snes::Tile8px::TILE_SIZE;

    const static Snes::Tile8px blankTile{};

    assert(image.size() == usize(animatedTileset.tileMap.width() * 8, animatedTileset.tileMap.height() * 8));

    const auto& animatedTilesFrames = animatedTileset.animatedTiles;

    const auto& staticTiles = animatedTileset.staticTiles;
    const auto& animatedTiles = frameIndex < animatedTilesFrames.size() ? animatedTilesFrames.at(frameIndex) : animatedTileset.staticTiles;
    const auto& map = animatedTileset.tileMap;
    const unsigned nPaletteColors = 1 << animatedTileset.staticTiles.bitDepthInt();
    const unsigned pixelMask = staticTiles.pixelMask();

    auto mapIt = map.cbegin();

    for (unsigned y = 0; y < image.height(); y += TILE_SIZE) {
        auto imageScanline = image.begin() + y * image.width();

        for (unsigned x = 0; x < image.height(); x += TILE_SIZE) {
            Snes::TilemapEntry tm = *mapIt++;

            const Snes::Tile8px& tile = [&]() {
                const auto c = tm.character();
                if (c < staticTiles.size()) {
                    return staticTiles.at(c);
                }
                else if (c < staticTiles.size() + animatedTiles.size()) {
                    return animatedTiles.at(c - staticTiles.size());
                }
                return blankTile;
            }();

            const uint8_t palOffset = tm.palette() * nPaletteColors;
            auto tileIt = tile.data().begin();

            for (unsigned ty = 0; ty < tile.TILE_SIZE; ty++) {
                unsigned fty = (tm.vFlip() == false) ? ty : TILE_SIZE - 1 - ty;

                auto imageBits = imageScanline + fty * image.width();

                for (unsigned tx = 0; tx < tile.TILE_SIZE; tx++) {
                    unsigned ftx = (tm.hFlip() == false) ? tx : TILE_SIZE - 1 - tx;

                    const auto tc = *tileIt & pixelMask;
                    imageBits[ftx] = tc != 0 ? (tc + palOffset) & 0xff : 0;

                    tileIt++;
                }
            }
            assert(tileIt == tile.data().end());

            imageScanline += TILE_SIZE;
        }
    }
    assert(mapIt == map.cend());
}

void MtTileset::reset()
{
    _tilesetImageFilenames.clear();
    _tilesetData = nullptr;
    _paletteData = nullptr;
    _tilesTextureValid = false;
    _textureValid = false;
    _interactiveTilesDataValid = false;
}

void MtTileset::setPaletteData(std::shared_ptr<const UnTech::Resources::PaletteData> pd)
{
    if (_paletteData != pd) {
        _paletteData = std::move(pd);

        if (_paletteData) {
            const Image pal = drawPaletteImage(*_paletteData);
            _palette.replace(pal);

            if (pal.size().height != _nPaletteFrames) {
                _paletteFrame = 0;
            }
            _nPaletteFrames = pal.size().height;
        }
        else {
            _paletteFrame = 0;
            _nPaletteFrames = 0;
        }
    }
}

void MtTileset::setTilesetData(const MetaTiles::MetaTileTilesetInput& input,
                               std::shared_ptr<const MetaTiles::MetaTileTilesetData> td)
{
    _tilesetImageFilenames = input.animationFrames.frameImageFilenames;

    if (_tilesetData != td) {
        _tilesetData = std::move(td);

        if (_tilesetData) {
            static grid<uint8_t> image(METATILE_SIZE_PX * TILESET_WIDTH, METATILE_SIZE_PX * TILESET_HEIGHT);

            const unsigned nFrames = std::min<unsigned>(_tilesetData->animatedTileset.nAnimatedFrames(), _tilesetFrames.size());

            if (_nTilesetFrames != nFrames) {
                _nTilesetFrames = nFrames;
            }
            _nTilesetFrames = nFrames;

            for (unsigned i = 0; i < nFrames; i++) {
                drawAnimatedTileset(image, _tilesetData->animatedTileset, i);
                _tilesetFrames.at(i).setData(image);
            }
        }
    }

    if (_tilesetData == nullptr) {
        if (_nTilesetFrames != _tilesetImageFilenames.size()) {
            _tilesetFrame = 0;
        }
        _nTilesetFrames = _tilesetImageFilenames.size();
    }

    _tilesTextureValid = false;
}

void MtTileset::setInteractiveTilesData(const MetaTiles::MetaTileTilesetInput& tileset,
                                        const Project::ProjectFile& projectFile,
                                        const Project::ProjectData& projectData)
{
    static Image image(TILESET_WIDTH, TILESET_HEIGHT);

    _textureValid = false;

    const auto interactiveTiles = projectData.interactiveTiles();

    if (!interactiveTiles) {
        _interactiveTilesDataValid = false;
        return;
    }

    image.fill(rgba(0, 0, 0, 0));

    auto* imgIt = image.data();
    for (const auto& ftName : tileset.tileFunctionTables) {
        if (ftName.isValid()) {
            auto it = interactiveTiles->tileFunctionMap.find(ftName);
            if (it != interactiveTiles->tileFunctionMap.end()) {
                const auto& itf = projectFile.interactiveTiles.getFunctionTable(it->second);

                *imgIt = itf.tint;
                imgIt->alpha = 64;
            }
        }

        imgIt++;
    }
    assert(imgIt == image.data() + image.dataSize());

    _interactiveTilesTexture.replace(image);
    _interactiveTilesDataValid = true;
}

}
