/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "metatile-tileset.h"
#include "models/common/bytevectorhelper.h"
#include "models/lz4/lz4.h"
#include "models/resources/resources.h"
#include <cassert>

namespace UnTech {
namespace MetaTiles {

bool MetaTileTilesetInput::validate(Resources::ErrorList& err) const
{
    bool valid = true;

    if (!name.isValid()) {
        err.addError("Expected metaTile tileset name");
        valid = false;
    }

    if (palettes.empty()) {
        err.addError("Expected at least one palette");
        valid = false;
    }

    if (!animationFrames.frameImages.empty()) {
        const auto& frameSize = animationFrames.frameImages.front().size();

        if (frameSize.width % METATILE_SIZE_PX != 0) {
            err.addError("Image width must be a multiple of 16");
            valid = false;
        }
        if (frameSize.height % METATILE_SIZE_PX != 0) {
            err.addError("Image height must be a multiple of 16");
            valid = false;
        }
    }

    if (valid) {
        valid &= animationFrames.validate(err);
    }

    return valid;
}

std::unique_ptr<MetaTileTilesetData> convertTileset(const MetaTileTilesetInput& input,
                                                    const Resources::ResourcesFile& resourcesFile,
                                                    Resources::ErrorList& err)
{
    bool valid = input.validate(err);
    if (!valid) {
        return nullptr;
    }

    const idstring& paletteName = input.palettes.front();
    const auto* palette = resourcesFile.getPalettePtr(paletteName);
    if (palette == nullptr) {
        err.addError("Cannot find palette: " + paletteName);
        return nullptr;
    }

    auto aniFrames = Resources::convertAnimationFrames(input.animationFrames, *palette, err);
    if (!aniFrames) {
        return nullptr;
    }

    auto ret = std::make_unique<MetaTileTilesetData>();
    ret->name = input.name;
    ret->palettes = input.palettes;
    ret->animatedTileset = std::move(aniFrames);

    valid = ret->validate(resourcesFile.metaTileEngineSettings, err);
    if (!valid) {
        return nullptr;
    }
    return ret;
}

bool MetaTileTilesetData::validate(const EngineSettings& settings, Resources::ErrorList& err) const
{
    bool valid = animatedTileset->validate(err);

    const auto& tileMap = animatedTileset->tileMap;

    if (tileMap.empty()) {
        err.addError("Expected at least one MetaTile");
        valid = false;
    }

    auto validateMax = [&](unsigned v, unsigned max, const char* msg) {
        if (v > max) {
            err.addError(msg + std::string(" (") + std::to_string(v) + ", max: " + std::to_string(max) + ")");
            valid = false;
        }
    };
    validateMax(tileMap.size() / 4, settings.nMetaTiles, "Too many MetaTiles");

    if (animatedTileset->mapWidth % 2 != 0) {
        err.addError("Tileset image width must be a multiple of 16");
        valid = false;
    }
    if (animatedTileset->mapHeight % 2 != 0) {
        err.addError("Tileset image height must be a multiple of 16");
        valid = false;
    }

    return valid;
}

std::vector<uint8_t> MetaTileTilesetData::convertTileMap(const EngineSettings& settings) const
{
    std::vector<uint8_t> out(settings.nMetaTiles * 2 * 4, 0);

    const unsigned& mapWidth = animatedTileset->mapWidth;
    const unsigned& mapHeight = animatedTileset->mapHeight;

    assert(mapWidth % 2 == 0);
    assert(mapHeight % 2 == 0);
    assert(animatedTileset->tileMap.size() == mapWidth * mapHeight);
    assert(animatedTileset->tileMap.size() <= out.size() / 2);

    for (unsigned q = 0; q < 4; q++) {
        auto mapIt = animatedTileset->tileMap.begin();
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
    std::vector<uint8_t> tmBlock = convertTileMap(settings);

    std::vector<uint8_t> out = lz4HcCompress(tmBlock);

    std::vector<uint8_t> atData = animatedTileset->exportAnimatedTileset();

    out.insert(out.end(), atData.begin(), atData.end());

    return out;
}
}
}
