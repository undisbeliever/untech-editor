/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "bit-depth.h"
#include "snescolor.h"
#include "tile.h"
#include "models/common/indexedimage.h"
#include <filesystem>

namespace UnTech::Snes {

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
        const IndexedImage& image, BitDepthSpecial bitDepth,
        const std::filesystem::path& tilesetFile, const std::filesystem::path& paletteFile);

private:
    const BitDepthSpecial _bitDepth;
    std::vector<Tile8px> _tileset;
    std::vector<SnesColor> _palette;

public:
    // Disabling copying/moving
    ImageToTileset(const ImageToTileset&) = delete;
    ImageToTileset(ImageToTileset&&) = delete;
    ImageToTileset& operator=(const ImageToTileset&) = delete;
    ImageToTileset& operator=(ImageToTileset&&) = delete;

    ~ImageToTileset() = default;

public:
    ImageToTileset(BitDepthSpecial bd);

    void writeTileset(const std::filesystem::path& filename) const;
    void writePalette(const std::filesystem::path& filename) const;

    [[nodiscard]] BitDepthSpecial bitDepth() const { return _bitDepth; }

    auto& tileset() { return _tileset; }
    [[nodiscard]] const auto& tileset() const { return _tileset; }

    auto& palette() { return _palette; }
    [[nodiscard]] const auto& palette() const { return _palette; }

    void process(const IndexedImage& image);

private:
    void processPalette(const IndexedImage& image);
    void processTileset(const IndexedImage& image);
};

}
