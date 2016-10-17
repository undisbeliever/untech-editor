#pragma once
#include "models/metasprite/metasprite.h"
#include "models/metasprite/spriteimporter.h"
#include <map>
#include <set>

namespace UnTech {
namespace MetaSprite {
namespace Utsi2UtmsPrivate {

namespace MS = UnTech::MetaSprite::MetaSprite;
namespace SI = UnTech::MetaSprite::SpriteImporter;

class PaletteConverter {
    const size_t PALETTE_COLORS = 16;

    const SI::FrameSet& siFrameSet;
    MS::FrameSet& msFrameSet;
    ErrorList& errorList;

    // Faster than std::unordered_map, only contains a max of 16 elements
    std::map<rgba, unsigned> _colorMap;

public:
    PaletteConverter(const SI::FrameSet& siFrameSet,
                     MS::FrameSet& msFrameSet,
                     ErrorList& errorList)
        : siFrameSet(siFrameSet)
        , msFrameSet(msFrameSet)
        , errorList(errorList)
        , _colorMap()
    {
    }

    const auto& colorMap() const { return _colorMap; }

    void process()
    {
        std::set<rgba> colors = getColorsFromImage();

        // Verify enough colors in tileset
        if (colors.size() > (PALETTE_COLORS)) {
            errorList.addError(siFrameSet, "Too many colors, expected a max of 16");
            return;
        }

        // remove transparent from colors list
        auto tIt = colors.find(siFrameSet.transparentColor);
        if (tIt != colors.end()) {
            colors.erase(tIt);
        }
        else {
            errorList.addWarning(siFrameSet, "Transparent color is not in frame objects");
        }

        // ::TODO handle user supplied palettes::

        buildAutomaticPalette(siFrameSet.transparentColor, colors);
    }

private:
    inline std::set<rgba> getColorsFromImage()
    {
        const Image& image = siFrameSet.image;

        std::set<rgba> colors;

        for (const auto& siFrameIt : siFrameSet.frames) {
            const SI::Frame& siFrame = siFrameIt.second;

            for (const SI::FrameObject& obj : siFrame.objects) {
                unsigned lx = siFrame.location.aabb.x + obj.location.x;
                unsigned ly = siFrame.location.aabb.y + obj.location.y;

                for (unsigned y = 0; y < obj.sizePx(); y++) {
                    const rgba* p = image.scanline(ly + y) + lx;

                    for (unsigned x = 0; x < obj.sizePx(); x++) {
                        colors.insert(*p++);
                    }
                }

                if (colors.size() > PALETTE_COLORS) {
                    return colors;
                }
            }
        }

        return colors;
    }

    inline void buildAutomaticPalette(const rgba transparentColor, std::set<rgba> colors)
    {
        // Store palette in MetaSprite
        msFrameSet.palettes.emplace_back();
        Snes::Palette4bpp& palette = msFrameSet.palettes.back();

        _colorMap.insert({ transparentColor, 0 });
        palette.color(0).setRgb(transparentColor);

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
