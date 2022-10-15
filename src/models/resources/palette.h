/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/idstring.h"
#include "models/snes/snescolor.h"
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace UnTech {
class ErrorList;
}

namespace UnTech::Resources {

struct PaletteInput {
    idstring name;

    std::filesystem::path paletteImageFilename;

    unsigned rowsPerFrame = 0;
    unsigned animationDelay = 0;

    bool skipFirstFrame = false;

    // returns 0 frames if input is invalid
    [[nodiscard]] unsigned nFrames() const;

    bool operator==(const PaletteInput&) const = default;
};

struct PaletteData {
    static constexpr unsigned MAX_PALETTE_BLOCK_SIZE = 32 * 128 * 2;
    static const int PALETTE_FORMAT_VERSION;

    // First palette in source image, even if `PaletteInput::skipFirstFrame` is set.
    std::vector<Snes::SnesColor> conversionPalette;

    std::vector<std::vector<Snes::SnesColor>> paletteFrames;
    unsigned animationDelay;

    [[nodiscard]] unsigned nAnimations() const { return paletteFrames.size(); }
    [[nodiscard]] unsigned colorsPerFrame() const;

    // PaleteData SHOULD BE valid before exporting
    [[nodiscard]] std::vector<uint8_t> exportSnesData() const;
};

// Will return a nullptr if PaletteInput or PaletteData is invalid
std::shared_ptr<const PaletteData>
convertPalette(const PaletteInput& input, ErrorList& err);

}
