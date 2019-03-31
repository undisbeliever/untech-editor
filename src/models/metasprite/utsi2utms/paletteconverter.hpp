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

        for (unsigned pal = 0; pal < siFrameSet.palette.nPalettes; pal++) {
            const unsigned yPos = image.size().height - pal * colorSize - 1;

            msFrameSet.palettes.emplace_back();
            Snes::Palette4bpp& palette = msFrameSet.palettes.back();

            for (unsigned i = 0; i < PALETTE_COLORS; i++) {
                const rgba c = image.getPixel(i * colorSize, yPos);

                palette.color(i).setRgb(c);

                if (pal == 0) {
                    _colorMap.insert({ c, i });
                }
            }
        }
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

        validateFirstUserSuppliedPalette();
    }

    inline void validateUserSuppliedPalette(unsigned pal) const
    {
        const unsigned colorSize = siFrameSet.palette.colorSize;
        const unsigned yPos = image.size().height - pal * colorSize - 1;

        const rgba* scanline = image.scanline(yPos);

        // ensure the scanlines of the palette equal

        for (unsigned l = 1; l < colorSize; l++) {
            const rgba* linetoTest = image.scanline(yPos - l);

            if (std::memcmp(scanline, linetoTest, sizeof(rgba) * colorSize * PALETTE_COLORS) != 0) {
                throw std::runtime_error("Custom Palette is invalid A");
            }
        }

        // ensure each of the palette colors are equally colored squares
        for (unsigned c = 0; c < PALETTE_COLORS; c++) {
            const rgba* imgBits = scanline + c * colorSize;

            for (unsigned i = 1; i < colorSize; i++) {
                if (imgBits[0] != imgBits[i]) {
                    throw std::runtime_error("Custom Palette is invalid");
                }
            }
        }

        // ensure first color is transparent
        if (scanline[0] != siFrameSet.transparentColor) {
            throw std::runtime_error("First color of custom palette " + std::to_string(pal)
                                     + " is not the transparent color");
        }
    }

    inline void validateFirstUserSuppliedPalette() const
    {
        const unsigned colorSize = siFrameSet.palette.colorSize;

        vectorset<rgba> colorSet = getColorsFromImage();
        assert(colorSet.size() <= PALETTE_COLORS - 1);

        const rgba* scanline = image.scanline(image.size().height - 1);

        for (unsigned i = 0; i < PALETTE_COLORS; i++) {
            const rgba c = scanline[i * colorSize];
            colorSet.erase(c);
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
