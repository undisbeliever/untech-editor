/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "palette.h"
#include "models/common/bytevectorhelper.h"
#include "models/common/errorlist.h"
#include "models/common/imagecache.h"
#include "models/common/iterators.h"
#include "models/lz4/lz4.h"
#include "models/snes/convert-snescolor.h"
#include <cassert>

namespace UnTech::Resources {

static bool validate(const PaletteData& input, ErrorList& err);

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

static bool validate(const PaletteInput& input, ErrorList& err)
{
    bool valid = true;

    const auto& paletteImage = ImageCache::loadPngImage(input.paletteImageFilename);
    const usize& imgSize = paletteImage->size();

    unsigned minImageHeight = input.skipFirstFrame ? input.rowsPerFrame * 2 : input.rowsPerFrame;
    unsigned colorsPerFrame = imgSize.width * input.rowsPerFrame;

    if (!input.name.isValid()) {
        err.addErrorString(u8"Expected palette name");
        valid = false;
    }
    if (paletteImage->empty()) {
        err.addErrorString(u8"Error loading palette image: ", paletteImage->errorString());
        valid = false;
    }
    if (input.rowsPerFrame == 0) {
        err.addErrorString(u8"Expected rowsPerFrame");
        valid = false;
    }
    if (imgSize.width != 4 && imgSize.width != 16) {
        err.addErrorString(u8"Palette image must be 4 or 16 pixels wide");
        valid = false;
    }
    if (imgSize.height < minImageHeight) {
        err.addErrorString(u8"Palette image must be a minimum of ", minImageHeight, u8"pixels tall");
        valid = false;
    }
    if (input.rowsPerFrame > 0 && imgSize.height % input.rowsPerFrame != 0) {
        err.addErrorString(u8"Palette image height must be a multiple of rowsPerFrame");
        valid = false;
    }
    if (colorsPerFrame > 256) {
        err.addErrorString(u8"Too many colors per palette frame");
        valid = false;
    }

    return valid;
}

std::shared_ptr<const PaletteData>
convertPalette(const PaletteInput& input, ErrorList& err)
{
    bool valid = validate(input, err);
    if (!valid) {
        return nullptr;
    }

    const auto& paletteImage = ImageCache::loadPngImage(input.paletteImageFilename);
    const usize& imgSize = paletteImage->size();

    const unsigned firstFrame = input.skipFirstFrame ? 1 : 0;
    const unsigned nFrames = (imgSize.height - 1) / input.rowsPerFrame + 1 - firstFrame;
    const unsigned colorsPerFrame = imgSize.width * input.rowsPerFrame;

    auto ret = std::make_shared<PaletteData>();
    ret->animationDelay = input.animationDelay;

    auto extractPalette = [&](std::vector<Snes::SnesColor>& frame, unsigned frameId) {
        frame.resize(colorsPerFrame);
        auto frameIt = frame.begin();

        const unsigned frameYOffset = frameId * input.rowsPerFrame;
        for (const auto fy : range(input.rowsPerFrame)) {
            auto imgBits = paletteImage->scanline(frameYOffset + fy);

            for (const auto& c : imgBits) {
                *frameIt++ = Snes::toSnesColor(c);
            }
        }
        assert(frameIt == frame.end());
    };

    ret->paletteFrames.resize(nFrames);
    for (auto [frameId, pf] : enumerate(ret->paletteFrames)) {
        extractPalette(pf, frameId + firstFrame);
    }

    if (input.skipFirstFrame) {
        ret->conversionPalette = ret->paletteFrames.front();
    }
    else {
        extractPalette(ret->conversionPalette, 0);
    }

    valid &= validate(*ret, err);

    if (!valid) {
        return nullptr;
    }

    return ret;
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

static bool validate(const PaletteData& input, ErrorList& err)
{
    bool valid = true;

    if (input.paletteFrames.empty()) {
        err.addErrorString(u8"Expected at least one palette frame");
        valid = false;
    }
    if (input.paletteFrames.size() > 255) {
        err.addErrorString(u8"Too many palette animations");
        valid = false;
    }

    unsigned blockSize = input.paletteFrames.size() * input.colorsPerFrame() * 2;
    if (blockSize > input.MAX_PALETTE_BLOCK_SIZE) {
        err.addErrorString(u8"Animation palette too large");
        valid = false;
    }

    if (input.paletteFrames.size() > 0) {
        if (input.animationDelay == 0) {
            err.addErrorString(u8"Expected animation delay");
            valid = false;
        }
        if (input.animationDelay > 0xffff) {
            err.addErrorString(u8"Animation delay too long");
            valid = false;
        }
    }

    for (const auto& pal : input.paletteFrames) {
        if (pal.size() != input.colorsPerFrame()) {
            err.addErrorString(u8"paletteFrames must contain the same number of colors in each frame");
            valid = false;
        }
    }

    return valid;
}

const int PaletteData::PALETTE_FORMAT_VERSION = 2;

std::vector<uint8_t> PaletteData::exportSnesData() const
{
    unsigned blockSize = paletteFrames.size() * colorsPerFrame() * 2;

    std::vector<uint8_t> block;
    block.reserve(blockSize);

    for (const auto& pal : paletteFrames) {
        assert(pal.size() == paletteFrames.front().size());
        for (const Snes::SnesColor& c : pal) {
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

}
