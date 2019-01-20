/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "metatile-tileset.h"
#include "models/common/bytevectorhelper.h"
#include "models/common/errorlist.h"
#include "models/common/imagecache.h"
#include "models/lz4/lz4.h"
#include "models/project/project.h"
#include <cassert>

namespace UnTech {

// test grid template class compiles
template class grid<uint16_t>;

namespace MetaTiles {

bool MetaTileTilesetInput::validate(ErrorList& err) const
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

    if (!animationFrames.frameImageFilenames.empty()) {
        const auto& firstImageFilename = animationFrames.frameImageFilenames.front();
        const auto& frameSize = ImageCache::loadPngImage(firstImageFilename)->size();

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

    if (scratchpad.width() > MAX_GRID_WIDTH || scratchpad.height() > MAX_GRID_HEIGHT) {
        err.addError("Scratchpad too large (maximum allowed size is " + std::to_string(MAX_GRID_WIDTH)
                     + "x" + std::to_string(MAX_GRID_HEIGHT) + ".");
    }

    return valid;
}

std::unique_ptr<MetaTileTilesetData> convertTileset(const MetaTileTilesetInput& input,
                                                    const Project::ProjectFile& projectFile,
                                                    ErrorList& err)
{
    bool valid = input.validate(err);
    if (!valid) {
        return nullptr;
    }

    const idstring& paletteName = input.palettes.front();
    const auto* palette = projectFile.palettes.find(paletteName);
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

    valid = ret->validate(projectFile.metaTileEngineSettings, err);
    if (!valid) {
        return nullptr;
    }
    return ret;
}

usize MetaTileTilesetData::sourceTileSize() const
{
    if (animatedTileset) {
        return usize(
            animatedTileset->tileMap.width() / 2,
            animatedTileset->tileMap.height() / 2);
    }
    else {
        return usize(0, 0);
    }
}

unsigned MetaTileTilesetData::nMetaTiles() const
{
    if (animatedTileset) {
        unsigned w = animatedTileset->tileMap.width() / 2;
        unsigned h = animatedTileset->tileMap.height() / 2;
        return w * h;
    }
    else {
        return 0;
    }
}

bool MetaTileTilesetData::validate(const EngineSettings& settings, ErrorList& err) const
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
    validateMax(tileMap.cellCount() / 4, settings.nMetaTiles, "Too many MetaTiles");

    if (animatedTileset->tileMap.width() % 2 != 0) {
        err.addError("Tileset image width must be a multiple of 16");
        valid = false;
    }
    if (animatedTileset->tileMap.height() % 2 != 0) {
        err.addError("Tileset image height must be a multiple of 16");
        valid = false;
    }

    return valid;
}

std::vector<uint8_t> MetaTileTilesetData::convertTileMap(const EngineSettings& settings) const
{
    std::vector<uint8_t> out(settings.nMetaTiles * 2 * 4, 0);

    const unsigned& mapWidth = animatedTileset->tileMap.width();
    const unsigned& mapHeight = animatedTileset->tileMap.height();

    assert(mapWidth % 2 == 0);
    assert(mapHeight % 2 == 0);
    assert(animatedTileset->tileMap.cellCount() == mapWidth * mapHeight);
    assert(animatedTileset->tileMap.cellCount() <= out.size() / 2);

    for (unsigned q = 0; q < 4; q++) {
        const unsigned xOffset = (q & 1) ? 1 : 0;
        const unsigned yOffset = (q & 2) ? 1 : 0;

        auto outIt = out.begin() + settings.nMetaTiles * q * 2;
        for (unsigned y = 0; y < mapHeight / 2; y++) {
            for (unsigned x = 0; x < mapWidth / 2; x++) {
                auto& tmCell = animatedTileset->tileMap.at(x * 2 + xOffset, y * 2 + yOffset);

                *outIt++ = tmCell.data & 0xff;
                *outIt++ = (tmCell.data >> 8) & 0xff;
            }
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
