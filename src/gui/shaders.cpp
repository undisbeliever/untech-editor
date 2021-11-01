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
#include "models/resources/drawing.h"
#include "models/snes/convert-snescolor.h"
#include <cassert>

#if defined(IMGUI_IMPL_SDL_OPENGL)
#include "opengl/shaders_opengl3.hpp"
#endif

namespace UnTech::Gui::Shaders {

static constexpr unsigned METATILE_SIZE_PX = MetaTiles::METATILE_SIZE_PX;
static constexpr unsigned TILESET_WIDTH = MetaTiles::TILESET_WIDTH;
static constexpr unsigned TILESET_HEIGHT = MetaTiles::TILESET_HEIGHT;

static void drawPaletteImage(Texture& texture, const Resources::PaletteData& palette)
{
    Image image(UINT8_MAX, palette.paletteFrames.size());
    image.fill(rgba(255, 0, 255, 255));

    for (auto [palIndex, palFrame] : enumerate(palette.paletteFrames)) {
        const unsigned nColors = std::min<unsigned>(palFrame.size(), UINT8_MAX);

        auto imgBits = image.scanline(palIndex);
        assert(imgBits.size() >= nColors);

        for (const auto i : range(nColors)) {
            imgBits[i] = Snes::toRgb(palFrame.at(i));
        }
    }

    texture.replace(image);
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
            drawPaletteImage(_palette, *_paletteData);

            if (_palette.height() != _nPaletteFrames) {
                _paletteFrame = 0;
            }
            _nPaletteFrames = _palette.height();
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

            for (const auto i : range(nFrames)) {
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

    auto imgBits = image.data();
    assert(imgBits.size() == tileset.tileFunctionTables.size());

    auto imgIt = imgBits.begin();
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
    assert(imgIt == imgBits.end());

    _interactiveTilesTexture.replace(image);
    _interactiveTilesDataValid = true;
}

}
