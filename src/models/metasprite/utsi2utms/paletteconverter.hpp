/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once
#include "models/common/errorlist.h"
#include "models/common/exceptions.h"
#include "models/common/iterators.h"
#include "models/common/stringstream.h"
#include "models/common/vectorset.h"
#include "models/metasprite/metasprite.h"
#include "models/metasprite/spriteimporter.h"
#include "models/snes/convert-snescolor.h"
#include <cstring>
#include <iomanip>
#include <map>

namespace UnTech::MetaSprite::Utsi2UtmsPrivate {

namespace MS = UnTech::MetaSprite::MetaSprite;
namespace SI = UnTech::MetaSprite::SpriteImporter;

// Faster than std::unordered_map, only contains a max of 16 elements
using ColorMapT = std::map<rgba, unsigned>;

static vectorset<rgba> getColorsFromImage(const SI::FrameSet& siFrameSet, const Image& image, ErrorList& errorList)
{
    assert(image.empty() == false);

    vectorset<rgba> colors;

    for (const SI::Frame& siFrame : siFrameSet.frames) {
        for (const SI::FrameObject& obj : siFrame.objects) {
            unsigned lx = siFrame.location.aabb.x + obj.location.x;
            unsigned ly = siFrame.location.aabb.y + obj.location.y;

            assert(lx + obj.sizePx() <= image.size().width);
            assert(ly + obj.sizePx() <= image.size().height);

            for (const auto y : range(obj.sizePx())) {
                const auto p = image.scanline(ly + y);

                for (const auto x : range(obj.sizePx())) {
                    const bool newColor = colors.insert(p[lx + x]);
                    if (newColor) {
                        if (colors.size() > PALETTE_COLORS) {
                            throw runtime_error("Too many colors, expected a maximum of 16 colors");
                        }
                    }
                }
            }
        }
    }

    // remove transparent from colors list
    auto tIt = colors.find(siFrameSet.transparentColor);
    if (tIt != colors.end()) {
        colors.erase(tIt);
    }
    else {
        errorList.addWarningString("Transparent color is not in frame objects");

        if (colors.size() > (PALETTE_COLORS - 1)) {
            throw runtime_error("Too many colors, expected a maximum of 15 colors after removing transparency");
        }
    }

    return colors;
}

// ASSUMES UserSuppliedPalette exists and is valid
static upoint palettePosition(const SI::UserSuppliedPalette& usp, const Image& image)
{
    using Position = SI::UserSuppliedPalette::Position;

    const usize imageSize = image.size();
    const usize paletteSize = usp.paletteSize();

    switch (usp.position) {
    case Position::TOP_LEFT:
        return upoint(0, 0);

    case Position::TOP_RIGHT:
        return upoint(imageSize.width - paletteSize.width, 0);

    case Position::BOTTOM_LEFT:
        return upoint(0, imageSize.height - paletteSize.height);

    case Position::BOTTOM_RIGHT:
        return upoint(imageSize.width - paletteSize.width, imageSize.height - paletteSize.height);
    }

    throw runtime_error("Invalid UserSuppliedPalette::Position");
}

static void validateUserSuppliedPalette(const SI::FrameSet& siFrameSet, const Image& image, const unsigned pal)
{
    const unsigned colorSize = siFrameSet.palette.colorSize;
    const upoint uspPos = palettePosition(siFrameSet.palette, image);
    const unsigned xPos = uspPos.x;
    const unsigned yPos = uspPos.y + pal * colorSize;

    assert(colorSize >= 1);
    assert(xPos + colorSize * PALETTE_COLORS <= image.size().width);
    assert(yPos + colorSize <= image.size().height);

    const auto startOfPalette = image.scanline(yPos).subspan(xPos, colorSize * PALETTE_COLORS);

    // ensure the pixel columns of the palette are equal
    for (const auto l : range(1, colorSize)) {
        const auto imgBitstoTest = image.scanline(yPos + l).subspan(xPos, colorSize * PALETTE_COLORS);

        if (not std::equal(startOfPalette.begin(), startOfPalette.end(), imgBitstoTest.begin())) {
            throw runtime_error("Custom Palette is invalid");
        }
    }

    // ensure each of the palette colors are equally colored squares
    for (const auto c : range(PALETTE_COLORS)) {
        const auto imgBits = startOfPalette.subspan(c * colorSize, colorSize);

        for (const auto i : range(1, colorSize)) {
            if (imgBits[0] != imgBits[i]) {
                throw runtime_error("Custom Palette is invalid");
            }
        }
    }

    // ensure first color is transparent
    if (startOfPalette[0] != siFrameSet.transparentColor) {
        throw runtime_error("First color of custom palette ", pal, " is not the transparent color");
    }
}

static void validateUserSuppliedPalettes(const SI::FrameSet& siFrameSet, const Image& image)
{
    const usize imageSize = image.size();
    const usize paletteSize = siFrameSet.palette.paletteSize();

    if (imageSize.width < paletteSize.width || imageSize.height < paletteSize.height) {
        throw runtime_error("Cannot load custom palette, image is too small");
    }

    for (const auto pal : range(siFrameSet.palette.nPalettes)) {
        validateUserSuppliedPalette(siFrameSet, image, pal);
    }
}

// Validate colorMap contains all the colors used in the colorSet
static void validateColorMap(const ColorMapT& colorMap, const unsigned paletteId, vectorset<rgba>&& colorSet)
{
    for (auto& it : colorMap) {
        colorSet.erase(it.first);
    }

    if (colorSet.size() > 0) {
        StringStream out(1024);
        out.write("Palette ", paletteId, " is invalid (missing");

        for (const rgba& c : colorSet) {
            out.write(" ", hex_6(c.rgbHex()));
        }
        out.write(")");

        throw runtime_error(out.takeString());
    }
}

static std::pair<std::vector<Snes::Palette4bpp>, ColorMapT>
buildUserSuppliedPalettes(const SI::FrameSet& siFrameSet, const Image& image, vectorset<rgba>&& colorsInImage)
{
    validateUserSuppliedPalettes(siFrameSet, image);

    const unsigned colorSize = siFrameSet.palette.colorSize;
    const upoint uspPos = palettePosition(siFrameSet.palette, image);

    std::pair<std::vector<Snes::Palette4bpp>, ColorMapT> ret;
    std::vector<Snes::Palette4bpp>& palettes = ret.first;
    ColorMapT& colorMap = ret.second;

    // This method allows the sprite-sheet palette to be different from the
    // first palette.
    //
    // This is for two reasons:
    //  1) The old palette converter used the bottom palette as the sprite
    //     sheet palette.
    //  2) Allow the user to easily change the order of the palettes without
    //     changing the sprite sheet.

    unsigned bestMatch = 0;
    unsigned bestMatchCount = 0;

    for (const auto pal : range(siFrameSet.palette.nPalettes)) {
        palettes.emplace_back();
        Snes::Palette4bpp& palette = palettes.back();

        unsigned xPos = uspPos.x;
        const unsigned yPos = uspPos.y + pal * colorSize;

        unsigned nMatching = 0;

        for (const auto i : range(PALETTE_COLORS)) {
            const rgba c = image.getPixel(xPos, yPos);
            xPos += colorSize;

            palette.at(i) = Snes::toSnesColor(c);

            if (colorsInImage.contains(c)) {
                nMatching++;
            }
        }

        if (nMatching > bestMatchCount) {
            bestMatch = pal;
            bestMatchCount = nMatching;
        }
    }

    // build _colorMap
    {
        unsigned xPos = uspPos.x;
        const unsigned yPos = uspPos.y + bestMatch * colorSize;

        for (const auto i : range(PALETTE_COLORS)) {
            const rgba c = image.getPixel(xPos, yPos);
            xPos += colorSize;

            colorMap.emplace(c, i);
        }
    }

    assert(colorMap.at(siFrameSet.transparentColor) == 0);
    validateColorMap(colorMap, bestMatch, std::move(colorsInImage));

    return ret;
}

static std::pair<std::vector<Snes::Palette4bpp>, ColorMapT> buildAutomaticPalette(const rgba& transparentColor,
                                                                                  vectorset<rgba>&& colors)
{
    assert(colors.size() <= PALETTE_COLORS - 1);

    std::pair<std::vector<Snes::Palette4bpp>, ColorMapT> ret;
    ret.first.emplace_back();

    Snes::Palette4bpp& palette = ret.first.front();
    ColorMapT& colorMap = ret.second;

    // Store palette in MetaSprite
    colorMap.insert({ transparentColor, 0 });
    palette.at(0) = Snes::toSnesColor(transparentColor);

    int i = 1;
    for (auto& c : colors) {
        colorMap.insert({ c, i });
        palette.at(i) = Snes::toSnesColor(c);
        i++;
    }

    return ret;
}

static std::pair<std::vector<Snes::Palette4bpp>, ColorMapT> buildPalette(const SI::FrameSet& siFrameSet, const Image& image, ErrorList& errorList)
{
    auto colorSet = getColorsFromImage(siFrameSet, image, errorList);

    if (siFrameSet.palette.usesUserSuppliedPalette()) {
        return buildUserSuppliedPalettes(siFrameSet, image, std::move(colorSet));
    }
    else {
        return buildAutomaticPalette(siFrameSet.transparentColor, std::move(colorSet));
    }
}

}
