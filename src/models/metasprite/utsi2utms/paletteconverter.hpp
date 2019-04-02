/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once
#include "models/common/errorlist.h"
#include "models/common/vectorset.h"
#include "models/metasprite/metasprite.h"
#include "models/metasprite/spriteimporter.h"
#include <cstring>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>

namespace UnTech {
namespace MetaSprite {
namespace Utsi2UtmsPrivate {

namespace MS = UnTech::MetaSprite::MetaSprite;
namespace SI = UnTech::MetaSprite::SpriteImporter;

class PaletteConverter {
    const SI::FrameSet& siFrameSet;
    const Image& image;
    MS::FrameSet& msFrameSet;
    ErrorList& errorList;

    // Faster than std::unordered_map, only contains a max of 16 elements
    std::map<rgba, unsigned> _colorMap;

public:
    PaletteConverter(const SI::FrameSet& siFrameSet,
                     const Image& image,
                     MS::FrameSet& msFrameSet,
                     ErrorList& errorList)
        : siFrameSet(siFrameSet)
        , image(image)
        , msFrameSet(msFrameSet)
        , errorList(errorList)
        , _colorMap()
    {
    }

    const auto& colorMap() const { return _colorMap; }

    void process()
    {
        try {
            if (siFrameSet.palette.usesUserSuppliedPalette()) {
                buildUserSuppliedPalettes();
            }
            else {
                buildAutomaticPalette();
            }
        }
        catch (const std::exception& ex) {
            errorList.addError(ex.what());
        }
    }

private:
    vectorset<rgba> getColorsFromImage() const
    {
        assert(image.empty() == false);

        vectorset<rgba> colors;

        for (const SI::Frame& siFrame : siFrameSet.frames) {
            for (const SI::FrameObject& obj : siFrame.objects) {
                unsigned lx = siFrame.location.aabb.x + obj.location.x;
                unsigned ly = siFrame.location.aabb.y + obj.location.y;

                assert(lx + obj.sizePx() <= image.size().width);
                assert(ly + obj.sizePx() <= image.size().height);

                for (unsigned y = 0; y < obj.sizePx(); y++) {
                    const rgba* p = image.scanline(ly + y) + lx;

                    for (unsigned x = 0; x < obj.sizePx(); x++) {
                        const bool newColor = colors.insert(*p++);
                        if (newColor) {
                            if (colors.size() > PALETTE_COLORS) {
                                throw std::runtime_error("Too many colors, expected a maximum of 16 colors");
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
            errorList.addWarning("Transparent color is not in frame objects");

            if (colors.size() > (PALETTE_COLORS - 1)) {
                throw std::runtime_error("Too many colors, expected a maximum of 15 colors after removing transparency");
            }
        }

        return colors;
    }

    inline void buildUserSuppliedPalettes()
    {
        validateUserSuppliedPalettes();

        const unsigned colorSize = siFrameSet.palette.colorSize;
        const upoint uspPos = palettePosition();
        const vectorset<rgba> colorSet = getColorsFromImage();

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

        for (unsigned pal = 0; pal < siFrameSet.palette.nPalettes; pal++) {
            msFrameSet.palettes.emplace_back();
            Snes::Palette4bpp& palette = msFrameSet.palettes.back();

            unsigned xPos = uspPos.x;
            const unsigned yPos = uspPos.y + pal * colorSize;

            unsigned nMatching = 0;

            for (unsigned i = 0; i < PALETTE_COLORS; i++) {
                const rgba c = image.getPixel(xPos, yPos);
                xPos += colorSize;

                palette.color(i).setRgb(c);

                if (colorSet.contains(c)) {
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

            for (unsigned i = 0; i < PALETTE_COLORS; i++) {
                const rgba c = image.getPixel(xPos, yPos);
                xPos += colorSize;

                _colorMap.emplace(c, i);
            }
        }

        validateColorMap(colorSet);
    }

    // ASSUMES UserSuppliedPalette exists and is valid
    inline upoint palettePosition() const
    {
        using Position = SI::UserSuppliedPalette::Position;

        const usize imageSize = image.size();
        const usize paletteSize = siFrameSet.palette.paletteSize();

        switch (siFrameSet.palette.position) {
        case Position::TOP_LEFT:
            return upoint(0, 0);

        case Position::TOP_RIGHT:
            return upoint(imageSize.width - paletteSize.width, 0);

        case Position::BOTTOM_LEFT:
            return upoint(0, imageSize.height - paletteSize.height);

        case Position::BOTTOM_RIGHT:
            return upoint(imageSize.width - paletteSize.width, imageSize.height - paletteSize.height);
        }

        throw std::runtime_error("Invalid UserSuppliedPalette::Position");
    }

    inline void validateUserSuppliedPalettes() const
    {
        const usize imageSize = image.size();
        const usize paletteSize = siFrameSet.palette.paletteSize();

        if (imageSize.width < paletteSize.width || imageSize.height < paletteSize.height) {
            throw std::runtime_error("Cannot load custom palette, image is too small");
        }

        for (unsigned pal = 0; pal < siFrameSet.palette.nPalettes; pal++) {
            validateUserSuppliedPalette(pal);
        }
    }

    inline void validateUserSuppliedPalette(unsigned pal) const
    {
        const unsigned colorSize = siFrameSet.palette.colorSize;
        const upoint uspPos = palettePosition();
        const unsigned xPos = uspPos.x;
        const unsigned yPos = uspPos.y + pal * colorSize;

        assert(xPos + colorSize * PALETTE_COLORS <= image.size().width);
        assert(yPos + colorSize <= image.size().height);

        const rgba* startOfPalette = image.scanline(yPos) + xPos;

        // ensure the pixel columns of the palette are equal
        for (unsigned l = 1; l < colorSize; l++) {
            const rgba* imgBitstoTest = image.scanline(yPos + l) + xPos;

            if (std::memcmp(startOfPalette, imgBitstoTest, sizeof(rgba) * colorSize * PALETTE_COLORS) != 0) {
                throw std::runtime_error("Custom Palette is invalid");
            }
        }

        // ensure each of the palette colors are equally colored squares
        for (unsigned c = 0; c <= PALETTE_COLORS; c++) {
            const rgba* imgBits = startOfPalette + c * colorSize;

            for (unsigned i = 1; i < colorSize; i++) {
                if (imgBits[0] != imgBits[i]) {
                    throw std::runtime_error("Custom Palette is invalid");
                }
            }
        }

        // ensure first color is transparent
        if (startOfPalette[0] != siFrameSet.transparentColor) {
            throw std::runtime_error("First color of custom palette " + std::to_string(pal)
                                     + " is not the transparent color");
        }
    }

    // Validate colorMap contains all the colors used in the colorSet
    inline void validateColorMap(vectorset<rgba> colorSet) const
    {
        assert(_colorMap.at(siFrameSet.transparentColor) == 0);

        for (auto& it : _colorMap) {
            colorSet.erase(it.first);
        }

        if (colorSet.size() > 0) {
            std::stringstream out;
            out << "Palette 0 is invalid (missing";

            for (const rgba& c : colorSet) {
                out << " " << std::hex << std::setfill('0') << std::setw(6) << c.rgb();
            }
            out << ")";

            throw std::runtime_error(out.str());
        }
    }

    inline void buildAutomaticPalette()
    {
        vectorset<rgba> colors = getColorsFromImage();
        assert(colors.size() <= PALETTE_COLORS - 1);

        // Store palette in MetaSprite
        msFrameSet.palettes.emplace_back();
        Snes::Palette4bpp& palette = msFrameSet.palettes.back();

        _colorMap.insert({ siFrameSet.transparentColor, 0 });
        palette.color(0).setRgb(siFrameSet.transparentColor);

        int i = 1;
        for (auto& c : colors) {
            _colorMap.insert({ c, i });
            palette.color(i).setRgb(c);
            i++;
        }
    }
};
}
}
}
