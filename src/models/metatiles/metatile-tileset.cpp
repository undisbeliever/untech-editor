/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "metatile-tileset.h"
#include "models/common/bytevectorhelper.h"
#include "models/common/errorlist.h"
#include "models/common/imagecache.h"
#include "models/lz4/lz4.h"
#include "models/project/project.h"
#include <climits>

namespace UnTech {

// test grid template class compiles
template class grid<uint16_t>;

namespace MetaTiles {

MetaTileTilesetInput::MetaTileTilesetInput()
{
    tileCollisions.fill(TileCollisionType::EMPTY);
}

bool MetaTileTilesetInput::validate(ErrorList& err) const
{
    bool valid = true;

    if (!name.isValid()) {
        err.addErrorString("Expected metaTile tileset name");
        valid = false;
    }

    if (palettes.empty()) {
        err.addErrorString("Expected at least one palette");
        valid = false;
    }

    if (!animationFrames.frameImageFilenames.empty()) {
        const auto& firstImageFilename = animationFrames.frameImageFilenames.front();
        const auto& frameSize = ImageCache::loadPngImage(firstImageFilename)->size();

        if (frameSize.width != TILESET_WIDTH * METATILE_SIZE_PX) {
            err.addErrorString("Image width must be ", TILESET_WIDTH * METATILE_SIZE_PX, "px.");
            valid = false;
        }
        if (frameSize.height != TILESET_HEIGHT * METATILE_SIZE_PX) {
            err.addErrorString("Image height must be ", TILESET_HEIGHT * METATILE_SIZE_PX, "px.");
            valid = false;
        }
    }

    if (valid) {
        valid &= animationFrames.validate(err);
    }

    if (scratchpad.width() > MAX_GRID_WIDTH || scratchpad.height() > MAX_GRID_HEIGHT) {
        err.addErrorString("Scratchpad too large (maximum allowed size is ", MAX_GRID_WIDTH, "x", MAX_GRID_HEIGHT, ".");
        valid = false;
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
    const auto palette = projectFile.palettes.find(paletteName);
    if (!palette) {
        err.addErrorString("Cannot find palette: ", paletteName);
        return nullptr;
    }

    auto aniFrames = Resources::convertAnimationFrames(input.animationFrames, palette(), err);
    if (!aniFrames) {
        return nullptr;
    }

    auto ret = std::make_unique<MetaTileTilesetData>();
    ret->name = input.name;
    ret->palettes = input.palettes;
    ret->animatedTileset = std::move(aniFrames);
    ret->tileCollisions = input.tileCollisions;

    valid = ret->validate(err);
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

bool MetaTileTilesetData::validate(ErrorList& err) const
{
    bool valid = animatedTileset->validate(err);

    if (animatedTileset->tileMap.empty()) {
        err.addErrorString("Expected at least one MetaTile");
        valid = false;
    }

    const unsigned tileMapMetaTiles = animatedTileset->tileMap.cellCount() / 4;
    if (tileMapMetaTiles != N_METATILES) {
        err.addErrorString("Expected ", N_METATILES, " MetaTiles (got ", tileMapMetaTiles, ").");
        valid = false;
    }

    if (animatedTileset->tileMap.width() != TILESET_WIDTH * 2
        || animatedTileset->tileMap.height() != TILESET_HEIGHT * 2) {

        err.addErrorString("Invalid tileset image size");
        valid = false;
    }

    return valid;
}

static uint8_t convertTileCollisionType(const TileCollisionType& tc)
{
    static_assert(N_TILE_COLLISONS == 17);

    if (unsigned(tc) < 16) {
        return (uint8_t(tc) & 0xf) << 4;
    }
    else if (tc == TileCollisionType::END_SLOPE) {
        return (uint8_t(TileCollisionType::SOLID) << 4) | 0x08;
    }

    // Should never be here
    abort();
}

std::vector<uint8_t> MetaTileTilesetData::convertTileMap() const
{
    constexpr size_t TILEMAP_SIZE = N_METATILES * 4 * 2;
    constexpr size_t COLLISON_SIZE = N_METATILES;

    std::vector<uint8_t> out(TILEMAP_SIZE + COLLISON_SIZE, 0);

    assert(animatedTileset->tileMap.width() == TILESET_WIDTH * 2);
    assert(animatedTileset->tileMap.height() == TILESET_HEIGHT * 2);
    assert(animatedTileset->tileMap.cellCount() == N_METATILES * 4);
    assert(animatedTileset->tileMap.cellCount() == TILEMAP_SIZE / 2);

    // tileset data
    for (unsigned q = 0; q < 4; q++) {
        const unsigned xOffset = (q & 1) ? 1 : 0;
        const unsigned yOffset = (q & 2) ? 1 : 0;

        auto outIt = out.begin() + N_METATILES * q * 2;
        for (unsigned y = 0; y < TILESET_HEIGHT; y++) {
            for (unsigned x = 0; x < TILESET_WIDTH; x++) {
                auto& tmCell = animatedTileset->tileMap.at(x * 2 + xOffset, y * 2 + yOffset);

                *outIt++ = tmCell.data & 0xff;
                *outIt++ = (tmCell.data >> 8) & 0xff;
            }
        }
        assert(outIt <= out.begin() + N_METATILES * (q + 1) * 2);
    }

    static_assert(sizeof(TileCollisionType) == 1);
    static_assert(sizeof(tileCollisions) == 256);

    // tile collision data
    auto outIt = out.begin() + TILEMAP_SIZE;
    for (const auto& tc : tileCollisions) {
        assert(unsigned(tc) < N_TILE_COLLISONS);
        *outIt++ = convertTileCollisionType(tc);
    }
    assert(outIt == out.end());

    return out;
}

const int MetaTileTilesetData::TILESET_FORMAT_VERSION = 7;

std::vector<uint8_t>
MetaTileTilesetData::exportMetaTileTileset() const
{
    std::vector<uint8_t> tmBlock = convertTileMap();

    std::vector<uint8_t> out = lz4HcCompress(tmBlock);

    std::vector<uint8_t> atData = animatedTileset->exportAnimatedTileset();

    out.insert(out.end(), atData.begin(), atData.end());

    return out;
}
}
}
