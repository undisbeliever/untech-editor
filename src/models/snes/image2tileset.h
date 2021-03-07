/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/indexedimage.h"
#include "models/snes/palette.h"
#include "models/snes/tileset.h"
#include <filesystem>

namespace UnTech {
namespace Snes {

/*
 * A very simple image to 8x8px tileset converter.
 *
 * Will only accept images with a single palette of the correct size.
 *
 * Throws an exception on error
 */
class ImageToTileset {
public:
    static void convertAndSave(
        const IndexedImage& image, int bitDepth,
        const std::filesystem::path& tilesetFile, const std::filesystem::path& paletteFile);

public:
    ImageToTileset(int bitDepth);
    ImageToTileset(const ImageToTileset&) = delete;

    void writeTileset(const std::filesystem::path& filename) const;
    void writePalette(const std::filesystem::path& filename) const;

    auto& tileset() { return _tileset; }
    const auto& tileset() const { return _tileset; }

    auto& palette() { return _palette; }
    const auto& palette() const { return _palette; }

    void process(const IndexedImage& image);

private:
    void processPalette(const IndexedImage& image);
    void processTileset(const IndexedImage& image);

private:
    Tileset8px _tileset;
    std::vector<SnesColor> _palette;
};
}
}
