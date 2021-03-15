/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "palette.h"
#include "models/common/grid.h"
#include "models/snes/tile-data.h"
#include "models/snes/tilemap.h"
#include <filesystem>
#include <vector>

namespace UnTech {
class ErrorList;
namespace Project {
template <typename T>
class DataStore;
}

namespace Resources {

struct BackgroundImageInput {
    idstring name;

    unsigned bitDepth = 4;

    std::filesystem::path imageFilename;

    // Palette used in tileset convertion, may not be the palette used on screen.
    idstring conversionPlette;

    unsigned firstPalette = 0;
    unsigned nPalettes = 8;

    bool defaultOrder = 0;

    bool isBitDepthValid() const;
    bool validate(ErrorList& err) const;

    bool operator==(const BackgroundImageInput& o) const
    {
        return name == o.name
               && bitDepth == o.bitDepth
               && imageFilename == o.imageFilename
               && conversionPlette == o.conversionPlette
               && firstPalette == o.firstPalette
               && nPalettes == o.nPalettes
               && defaultOrder == o.defaultOrder;
    }
    bool operator!=(const BackgroundImageInput& o) const { return !(*this == o); }
};

struct BackgroundImageData {
    constexpr static unsigned MAX_N_TILEMAPS = 16;
    constexpr static unsigned MAX_SNES_TILES = 1024;
    constexpr static unsigned MAX_UNCOMPRESSED_DATA_SIZE = 48 * 1024;

    const static int BACKGROUND_IMAGE_FORMAT_VERSION;

    unsigned conversionPaletteIndex;

    unsigned bitDepth;
    std::vector<Snes::Tile8px> tiles;
    grid<Snes::TilemapEntry> tileMap;

    bool validate(ErrorList& err) const;

    unsigned nTilemaps() const;

    unsigned uncompressedDataSize() const;
    unsigned tilesetDataSize() const;
    unsigned tilemapDataSize() const;

    bool tilemapHorizontalMirroring() const { return tileMap.width() > 32; }
    bool tilemapVerticalMirroring() const { return tileMap.height() > 32; }

    std::vector<uint8_t> exportSnesData() const;
};

std::shared_ptr<const BackgroundImageData>
convertBackgroundImage(const BackgroundImageInput& input,
                       const Project::DataStore<PaletteData>& projectDataStore,
                       ErrorList& err);
}
}
