/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "palette.h"
#include "models/common/bytevectorhelper.h"
#include "models/common/imagecache.h"
#include "models/lz4/lz4.h"
#include <cassert>

using namespace UnTech;
using namespace UnTech::Resources;
using namespace UnTech::Snes;

// PaletteInput
// ============

unsigned PaletteInput::nFrames() const
{
    const auto& paletteImage = ImageCache::loadPngImage(paletteImageFilename);
    const usize& imgSize = paletteImage->size();

    unsigned minImageHeight = skipFirstFrame ? rowsPerFrame * 2 : rowsPerFrame;
    unsigned colorsPerFrame = imgSize.width * rowsPerFrame;

    if (paletteImage->empty()) {
        return 0;
    }
    if (rowsPerFrame == 0) {
        return 0;
    }
    if (imgSize.width != 4 && imgSize.width != 16) {
        return 0;
    }
    if (imgSize.height < minImageHeight) {
        return 0;
    }
    if (imgSize.height % rowsPerFrame != 0) {
        return 0;
    }
    if (colorsPerFrame > 256) {
        return 0;
    }

    return imgSize.height / rowsPerFrame;
}

bool PaletteInput::validate(ErrorList& err) const
{
    bool valid = true;

    const auto& paletteImage = ImageCache::loadPngImage(paletteImageFilename);
    const usize& imgSize = paletteImage->size();

    unsigned minImageHeight = skipFirstFrame ? rowsPerFrame * 2 : rowsPerFrame;
    unsigned colorsPerFrame = imgSize.width * rowsPerFrame;

    if (!name.isValid()) {
        err.addError("Expected palette name");
        valid = false;
    }
    if (paletteImage->empty()) {
        err.addError("Error loading palette image: " + paletteImage->errorString());
        valid = false;
    }
    if (rowsPerFrame == 0) {
        err.addError("Expected rowsPerFrame");
        valid = false;
    }
    if (imgSize.width != 4 && imgSize.width != 16) {
        err.addError("Palette image must be 4 or 16 pixels wide");
        valid = false;
    }
    if (imgSize.height < minImageHeight) {
        err.addError("Palette image must be a minimum of "
                     + std::to_string(minImageHeight) + "pixels tall");
        valid = false;
    }
    if (rowsPerFrame > 0 && imgSize.height % rowsPerFrame != 0) {
        err.addError("Palette image height must be a multiple of rowsPerFrame");
        valid = false;
    }
    if (colorsPerFrame > 256) {
        err.addError("Too many colors per palette frame");
        valid = false;
    }

    return valid;
}

std::unique_ptr<PaletteData> Resources::convertPalette(const PaletteInput& input, ErrorList& err)
{
    bool valid = input.validate(err);
    if (!valid) {
        return nullptr;
    }

    const auto& paletteImage = ImageCache::loadPngImage(input.paletteImageFilename);
    const usize& imgSize = paletteImage->size();

    unsigned startingY = input.skipFirstFrame ? input.rowsPerFrame : 0;
    unsigned colorsPerFrame = imgSize.width * input.rowsPerFrame;

    std::unique_ptr<PaletteData> ret = std::make_unique<PaletteData>();
    ret->name = input.name;
    ret->animationDelay = input.animationDelay;

    for (unsigned py = startingY; py < imgSize.height; py += input.rowsPerFrame) {
        ret->paletteFrames.emplace_back(colorsPerFrame);
        std::vector<SnesColor>& frame = ret->paletteFrames.back();
        auto frameIt = frame.begin();

        for (unsigned fy = 0; fy < input.rowsPerFrame; fy++) {
            const auto* imgBits = paletteImage->scanline(py + fy);

            for (unsigned fx = 0; fx < imgSize.width; fx++) {
                *frameIt++ = imgBits[fx];
            }
        }
        assert(frameIt == frame.end());
    }

    valid = ret->validate(err);
    if (!valid) {
        return nullptr;
    }

    return ret;
}

std::vector<Snes::SnesColor>
Resources::extractFirstPalette(const PaletteInput& input, unsigned bitDepth, ErrorList& err)
{
    bool valid = input.validate(err);

    if (bitDepth == 0 || bitDepth > 8) {
        err.addError("Invalid bit-depth");
        valid = false;
    }

    unsigned maxColors = 128;
    if (bitDepth <= 2) {
        maxColors = 32;
    }
    if (bitDepth > 4) {
        maxColors = 256;
    }

    const auto& img = ImageCache::loadPngImage(input.paletteImageFilename);

    unsigned nColumns = img->size().width;
    unsigned nRows = input.rowsPerFrame;
    if (nRows * nColumns > maxColors) {
        nRows = maxColors / nColumns;
    }

    if (img->size().height < nRows) {
        err.addError("Palette image must be a minimum of "
                     + std::to_string(nRows) + "pixels tall");
        valid = false;
    }

    if (!valid) {
        return std::vector<SnesColor>();
    }

    std::vector<Snes::SnesColor> palette(nRows * nColumns, SnesColor::invalidColor());
    assert(palette.size() < maxColors);

    auto pIt = palette.begin();

    for (unsigned y = 0; y < nRows; y++) {
        const rgba* scanline = img->scanline(y);

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

bool PaletteData::validate(ErrorList& err) const
{
    bool valid = true;

    if (paletteFrames.empty()) {
        err.addError("Expected at least one palette frame");
        valid = false;
    }
    if (paletteFrames.size() > 255) {
        err.addError("Too many palette animations");
        valid = false;
    }

    unsigned blockSize = paletteFrames.size() * colorsPerFrame() * 2;
    if (blockSize > MAX_PALETTE_BLOCK_SIZE) {
        err.addError("Animation palette too large");
        valid = false;
    }

    if (paletteFrames.size() > 0) {
        if (animationDelay == 0) {
            err.addError("Expected animation delay");
            valid = false;
        }
        if (animationDelay > 0xffff) {
            err.addError("Animation delay too long");
            valid = false;
        }
    }

    for (const auto& pal : paletteFrames) {
        if (pal.size() != colorsPerFrame()) {
            err.addError("paletteFrames must contain the same number of colors in each frame");
            valid = false;
        }
    }

    return valid;
}

const int PaletteData::PALETTE_FORMAT_VERSION = 1;

std::vector<uint8_t> PaletteData::exportPalette() const
{
    unsigned blockSize = paletteFrames.size() * colorsPerFrame() * 2;

    std::vector<uint8_t> block;
    block.reserve(blockSize);

    for (const auto& pal : paletteFrames) {
        assert(pal.size() == paletteFrames.front().size());
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
