/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "palette.h"
#include "models/common/grid.h"
#include "models/snes/tilemap.h"
#include "models/snes/tileset.h"
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

    idstring name;

    unsigned conversionPaletteIndex;

    Snes::Tileset8px tiles;
    grid<Snes::TilemapEntry> tileMap;

    BackgroundImageData(int bitDepth)
        : tiles(bitDepth)
    {
    }

    bool validate(ErrorList& err) const;

    unsigned nTilemaps() const;

    unsigned uncompressedDataSize() const;
    unsigned tilemapDataSize() const;

    std::vector<uint8_t> exportSnesData() const;
};

std::unique_ptr<BackgroundImageData>
convertBackgroundImage(const BackgroundImageInput& input,
                       const Project::DataStore<PaletteData>& projectDataStore,
                       ErrorList& err);
}
}
