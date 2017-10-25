/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "palette.h"
#include "models/common/bytevectorhelper.h"
#include "models/common/validatorhelper.h"
#include "models/lz4/lz4.h"
#include <cassert>

using namespace UnTech;
using namespace UnTech::Resources;
using namespace UnTech::Snes;

// PaletteInput
// ============

void PaletteInput::validate() const
{
    const usize& imgSize = paletteImage.size();

    unsigned minImageHeight = skipFirstFrame ? rowsPerFrame * 2 : rowsPerFrame;
    unsigned colorsPerFrame = imgSize.width * rowsPerFrame;

    if (!name.isValid()) {
        throw std::runtime_error("Expected palette name");
    }
    if (paletteImage.empty()) {
        throw std::runtime_error("Error loading palette image: " + paletteImage.errorString());
    }
    if (rowsPerFrame == 0) {
        throw std::runtime_error("Expected rowsPerFrame");
    }
    if (imgSize.width != 4 && imgSize.width != 16) {
        throw std::runtime_error("Palette image must be 4 or 16 pixels wide");
    }
    if (imgSize.height < minImageHeight) {
        throw std::runtime_error("Palette image must be a minimum of "
                                 + std::to_string(minImageHeight) + "pixels tall");
    }
    if (imgSize.height % rowsPerFrame != 0) {
        throw std::runtime_error("Palette image height must be a multiple of rowsPerFrame");
    }
    if (colorsPerFrame > 256) {
        throw std::runtime_error("Too many colors per palette frame");
    }
}

PaletteData Resources::convertPalette(const PaletteInput& input)
{
    input.validate();

    const usize& imgSize = input.paletteImage.size();

    unsigned startingY = input.skipFirstFrame ? input.rowsPerFrame : 0;
    unsigned colorsPerFrame = imgSize.width * input.rowsPerFrame;

    PaletteData ret;
    ret.name = input.name;
    ret.animationDelay = input.animationDelay;

    for (unsigned py = startingY; py < imgSize.height; py += input.rowsPerFrame) {
        ret.paletteFrames.emplace_back(colorsPerFrame);
        std::vector<SnesColor>& frame = ret.paletteFrames.back();
        auto frameIt = frame.begin();

        for (unsigned fy = 0; fy < input.rowsPerFrame; fy++) {
            const auto* imgBits = input.paletteImage.scanline(py + fy);

            for (unsigned fx = 0; fx < imgSize.width; fx++) {
                *frameIt++ = imgBits[fx];
            }
        }
        assert(frameIt == frame.end());
    }

    return ret;
}

std::vector<Snes::SnesColor>
Resources::extractFirstPalette(const PaletteInput& input, unsigned bitDepth)
{
    input.validate();

    if (bitDepth == 0 || bitDepth > 8) {
        throw std::invalid_argument("Invalid bit-depth");
    }

    unsigned maxColors = 128;
    if (bitDepth <= 2) {
        maxColors = 32;
    }
    if (bitDepth > 4) {
        maxColors = 256;
    }

    const Image& img = input.paletteImage;

    unsigned nColumns = img.size().width;
    unsigned nRows = input.rowsPerFrame;
    if (nRows * nColumns > maxColors) {
        nRows = maxColors / nColumns;
    }

    if (img.size().height < nRows) {
        throw std::runtime_error("Palette image must be a minimum of "
                                 + std::to_string(nRows) + "pixels tall");
    }

    std::vector<Snes::SnesColor> palette(nRows * nColumns, SnesColor::invalidColor());
    assert(palette.size() < maxColors);

    auto pIt = palette.begin();

    for (unsigned y = 0; y < nRows; y++) {
        const rgba* scanline = img.scanline(y);

        for (unsigned x = 0; x < nColumns; x++) {
            *pIt++ = *scanline++;
        }
    }
    assert(pIt == palette.end());

    return palette;
}

// PaletteData
// ===========

unsigned PaletteData::colorsPerFrame() const
{
    if (!paletteFrames.empty()) {
        return paletteFrames.front().size();
    }
    else {
        return 0;
    }
}

void PaletteData::validate() const
{
    validateNotEmpty(paletteFrames, "Expected at least one palette frame");
    validateMax(paletteFrames.size(), 255, "Too many palette animations");

    unsigned blockSize = paletteFrames.size() * colorsPerFrame() * 2;
    validateMax(blockSize, MAX_PALETTE_BLOCK_SIZE, "Animation palette too large");

    if (paletteFrames.size() > 0) {
        if (animationDelay == 0) {
            throw std::runtime_error("Expected animation delay");
        }
        validateMax(animationDelay, 0xffff, "Animation delay too long");
    }

    for (const auto& pal : paletteFrames) {
        if (pal.size() != colorsPerFrame()) {
            throw std::runtime_error("paletteFrames must contain the same number of colors in each frame");
        }
    }
}

const unsigned PaletteData::PALETTE_FORMAT_VERSION = 1;

std::vector<uint8_t> PaletteData::exportPalette() const
{
    validate();

    unsigned blockSize = paletteFrames.size() * colorsPerFrame() * 2;

    std::vector<uint8_t> block;
    block.reserve(blockSize);

    for (const auto& pal : paletteFrames) {
        for (const SnesColor& c : pal) {
            writeUint16(block, c.data());
        }
    }
    assert(block.size() == blockSize);
    block = lz4HcCompress(block);

    std::vector<uint8_t> out;
    out.reserve(block.size() + 3);

    if (nAnimations() > 1) {
        writeUint8(out, nAnimations());
        writeUint16(out, animationDelay);
    }
    else {
        writeUint8(out, 0);
    }

    out.insert(out.end(), block.begin(), block.end());

    return out;
}
