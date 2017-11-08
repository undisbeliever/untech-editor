/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "metatile-tileset.h"
#include "models/common/bytevectorhelper.h"
#include "models/common/validatorhelper.h"
#include "models/lz4/lz4.h"
#include "models/resources/resources.h"
#include <cassert>

namespace UnTech {
namespace MetaTiles {

void MetaTileTilesetInput::validate() const
{
    if (!name.isValid()) {
        throw std::runtime_error("Expected metaTile tileset name");
    }

    validateNotEmpty(palettes, "Expected at least one palette");

    if (!animationFrames.frameImages.empty()) {
        const auto& frameSize = animationFrames.frameImages.front().size();

        if (frameSize.width % METATILE_SIZE_PX != 0) {
            throw std::runtime_error("Image width must be a multiple of 16");
        }
        if (frameSize.height % METATILE_SIZE_PX != 0) {
            throw std::runtime_error("Image height must be a multiple of 16");
        }
    }

    animationFrames.validate();
}

MetaTileTilesetData convertTileset(const MetaTileTilesetInput& input,
                                   const Resources::ResourcesFile& resourcesFile)
{
    input.validate();

    const auto& palette = resourcesFile.getPalette(input.palettes.front());

    return MetaTileTilesetData{
        input.name,
        input.palettes,
        Resources::convertAnimationFrames(input.animationFrames, palette)
    };
}

void MetaTileTilesetData::validate(const EngineSettings& settings) const
{
    animatedTileset.validate();

    const auto& tileMap = animatedTileset.tileMap;

    validateNotEmpty(tileMap, "Expected at least one MetaTile");
    validateMax(tileMap.size() / 4, settings.nMetaTiles, "Too many MetaTiles");

    if (animatedTileset.mapWidth % 2 != 0) {
        throw std::runtime_error("Tileset image width must be a multiple of 16");
    }
    if (animatedTileset.mapHeight % 2 != 0) {
        throw std::runtime_error("Tileset image height must be a multiple of 16");
    }
}

std::vector<uint8_t> MetaTileTilesetData::convertTileMap(const EngineSettings& settings) const
{
    validate(settings);

    std::vector<uint8_t> out(settings.nMetaTiles * 2 * 4, 0);

    const unsigned& mapWidth = animatedTileset.mapWidth;
    const unsigned& mapHeight = animatedTileset.mapHeight;

    assert(animatedTileset.tileMap.size() == mapWidth * mapHeight);
    assert(animatedTileset.tileMap.size() <= out.size() / 2);

    for (unsigned q = 0; q < 4; q++) {
        auto mapIt = animatedTileset.tileMap.begin();
        if (q & 1) {
            mapIt += 1;
        }
        if (q & 2) {
            mapIt += mapWidth;
        }

        auto outIt = out.begin() + settings.nMetaTiles * q * 2;
        for (unsigned y = 0; y < mapHeight / 2; y++) {
            for (unsigned x = 0; x < mapWidth / 2; x++) {
                *outIt++ = mapIt->data & 0xff;
                *outIt++ = (mapIt->data >> 8) & 0xff;

                mapIt += 2;
            }
            mapIt += mapWidth;
        }
        assert(outIt <= out.begin() + settings.nMetaTiles * (q + 1) * 2);
    }

    return out;
}

const int MetaTileTilesetData::TILESET_FORMAT_VERSION = 1;

std::vector<uint8_t>
MetaTileTilesetData::exportMetaTileTileset(const EngineSettings& settings) const
{
    validate(settings);

    std::vector<uint8_t> tmBlock = convertTileMap(settings);

    std::vector<uint8_t> out = lz4HcCompress(tmBlock);

    std::vector<uint8_t> atData = animatedTileset.exportAnimatedTileset();

    out.insert(out.end(), atData.begin(), atData.end());

    return out;
}
}
}
