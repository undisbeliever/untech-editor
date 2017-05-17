/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/indexedimage.h"
#include "models/snes/palette.h"
#include "models/snes/tileset.h"

namespace UnTech {
namespace Snes {

/*
 * A very simple image to 8x8px tileset converter.
 *
 * Will only accept images with a single palette of the correct size.
 *
 * Throws an exception on error
 */
template <size_t BIT_DEPTH>
class ImageToTileset {
public:
    static void convertAndSave(
        const IndexedImage& image,
        const std::string& tilesetFile, const std::string& paletteFile);

public:
    ImageToTileset() = default;
    ImageToTileset(const ImageToTileset&) = delete;

    void writeTileset(const std::string& filename) const;
    void writePalette(const std::string& filename) const;

    auto& tileset() { return _tileset; }
    const auto& tileset() const { return _tileset; }

    auto& palette() { return _palette; }
    const auto& palette() const { return _palette; }

    void process(const IndexedImage& image);

private:
    void processPalette(const IndexedImage& image);
    void processTileset(const IndexedImage& image);

private:
    Tileset8px<BIT_DEPTH> _tileset;
    Palette<BIT_DEPTH> _palette;
};
}
}
