/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "error-list.h"
#include "models/common/idstring.h"
#include "models/common/image.h"
#include "models/snes/palette.h"
#include <memory>
#include <string>
#include <vector>

namespace UnTech {
namespace Resources {

struct PaletteInput {
    idstring name;

    std::string paletteImageFilename;
    Image paletteImage;

    unsigned rowsPerFrame = 0;
    unsigned animationDelay = 0;

    bool skipFirstFrame = false;

    bool validate(ErrorList& err) const;

    bool operator==(const PaletteInput& o) const
    {
        return paletteImageFilename == o.paletteImageFilename
               && rowsPerFrame == o.rowsPerFrame
               && animationDelay == o.animationDelay
               && skipFirstFrame == o.skipFirstFrame;
    }
    bool operator!=(const PaletteInput& o) const { return !(*this == o); }
};

struct PaletteData {
    static constexpr unsigned MAX_PALETTE_BLOCK_SIZE = 32 * 128 * 2;
    static const int PALETTE_FORMAT_VERSION;

    idstring name;

    std::vector<std::vector<Snes::SnesColor>> paletteFrames;
    unsigned animationDelay;

    unsigned nAnimations() const { return paletteFrames.size(); }
    unsigned colorsPerFrame() const;

    bool validate(ErrorList& err) const;

    // PaleteData SHOULD BE valid before exporting
    std::vector<uint8_t> exportPalette() const;
};

// Will return a nullptr if PaletteInput or PaletteData is invalid
std::unique_ptr<PaletteData> convertPalette(const PaletteInput& input, ErrorList& err);

// Extracts the first palette from the palette image, even `skipFirstFrame` is set.
//
// The output palette will contain:
//  * a multiple of 4, 32, or 256 colors, depending on the bitDepth argument.
//  * contain at most 32, 128 or 256 colors, depending on the bitDepth argument.
std::vector<Snes::SnesColor> extractFirstPalette(const PaletteInput& input, unsigned bitDepth, ErrorList& err);
}
}
