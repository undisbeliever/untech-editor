/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "metatile-tileset.h"
#include "interactive-tiles.h"
#include "metatiles-error.h"
#include "models/common/bytevectorhelper.h"
#include "models/common/imagecache.h"
#include "models/common/iterators.h"
#include "models/lz4/lz4.h"
#include <climits>

namespace UnTech {

// test grid template class compiles
template class grid<uint16_t>;

namespace MetaTiles {

static_assert(sizeof(TilePriorities) == N_METATILES * 4 / 8);

template <typename... Args>
std::unique_ptr<TilesetError> tileError(unsigned index, const Args... msg)
{
    return std::make_unique<TilesetError>(TilesetErrorType::TILE, index,
                                          stringBuilder("Tile ", index, ": ", msg...));
}

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

std::shared_ptr<const MetaTileTilesetData>
convertTileset(const MetaTileTilesetInput& input,
               const Project::DataStore<Resources::PaletteData>& paletteDataStore, const InteractiveTilesData& interactiveTilesData,
               ErrorList& err)

{
    bool valid = input.validate(err);
    if (!valid) {
        return nullptr;
    }

    auto addTileError = [&](const unsigned index, const auto&... msg) {
        err.addError(tileError(index, msg...));
        valid = false;
    };

    auto aniFrames = Resources::convertAnimationFrames(input.animationFrames, paletteDataStore, err);
    if (!aniFrames) {
        return nullptr;
    }

    // Set tile priorities
    {
        auto it = aniFrames->tileMap.begin();
        for (const uint8_t tpData : input.tilePriorities.data) {
            for (const auto bit : range(8)) {
                it->setOrder(tpData & (1 << bit));
                it++;
            }
        }
        assert(it == aniFrames->tileMap.end());
    }

    auto ret = std::make_shared<MetaTileTilesetData>(std::move(*aniFrames));
    ret->palettes = input.palettes;
    ret->tileCollisions = input.tileCollisions;

    ret->crumblingTiles = input.crumblingTiles;
    for (CrumblingTileChain& chain : ret->crumblingTiles) {
        if (chain.hasThirdTransition() == false) {
            chain.secondDelay = CrumblingTileChain::NO_THIRD_TRANSITION;
            chain.thirdTileId = 0;
        }
    }

    // Set Tile Function Tables
    for (const auto [i, ft] : const_enumerate(input.tileFunctionTables)) {
        const auto it = interactiveTilesData.tileFunctionMap.find(ft);
        if (it != interactiveTilesData.tileFunctionMap.end()) {
            ret->tileFunctionTables.at(i) = it->second;
        }
        else {
            addTileError(i, "Unknown Interactive Tile Function Table ", ft);
        }
    }

    valid &= ret->validate(err);
    if (!valid) {
        return nullptr;
    }
    return ret;
}

MetaTileTilesetData::MetaTileTilesetData(Resources::AnimatedTilesetData&& animatedTilesetData)
    : animatedTileset(std::move(animatedTilesetData))
{
}

usize MetaTileTilesetData::sourceTileSize() const
{
    return usize(
        animatedTileset.tileMap.width() / 2,
        animatedTileset.tileMap.height() / 2);
}

bool MetaTileTilesetData::validate(ErrorList& err) const
{
    bool valid = animatedTileset.validate(err);

    if (animatedTileset.tileMap.empty()) {
        err.addErrorString("Expected at least one MetaTile");
        valid = false;
    }

    const unsigned tileMapMetaTiles = animatedTileset.tileMap.cellCount() / 4;
    if (tileMapMetaTiles != N_METATILES) {
        err.addErrorString("Expected ", N_METATILES, " MetaTiles (got ", tileMapMetaTiles, ").");
        valid = false;
    }

    if (animatedTileset.tileMap.width() != TILESET_WIDTH * 2
        || animatedTileset.tileMap.height() != TILESET_HEIGHT * 2) {

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

std::vector<uint8_t> MetaTileTilesetData::convertTileset() const
{
    constexpr size_t TILEMAP_SIZE = N_METATILES * 4 * 2;
    constexpr size_t FOOTER_SIZE = 14;

    std::vector<uint8_t> out(TILEMAP_SIZE + N_METATILES * 2 + FOOTER_SIZE, 0);

    assert(animatedTileset.tileMap.width() == TILESET_WIDTH * 2);
    assert(animatedTileset.tileMap.height() == TILESET_HEIGHT * 2);
    assert(animatedTileset.tileMap.cellCount() == N_METATILES * 4);
    assert(animatedTileset.tileMap.cellCount() == TILEMAP_SIZE / 2);

    // tileset data
    for (const auto q : range(4)) {
        const unsigned xOffset = (q & 1) ? 1 : 0;
        const unsigned yOffset = (q & 2) ? 1 : 0;

        auto outIt = out.begin() + N_METATILES * q * 2;
        for (const auto y : range(TILESET_HEIGHT)) {
            for (const auto x : range(TILESET_HEIGHT)) {
                auto& tmCell = animatedTileset.tileMap.at(x * 2 + xOffset, y * 2 + yOffset);

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

    // Function Table data
    for (const auto& tf : tileFunctionTables) {
        static_assert(MAX_INTERACTIVE_TILE_FUNCTION_TABLES < UINT8_MAX / 2);
        assert(tf < MAX_INTERACTIVE_TILE_FUNCTION_TABLES);
        *outIt++ = tf * 2;
    }

    // Footer
    {
        for (auto& chain : crumblingTiles) {
            *outIt++ = chain.firstTileId;
            *outIt++ = chain.secondTileId;
            *outIt++ = chain.thirdTileId;

            *outIt++ = chain.firstDelay & 0xff;
            *outIt++ = chain.firstDelay >> 8;

            *outIt++ = chain.secondDelay & 0xff;
            *outIt++ = chain.secondDelay >> 8;
        }
    }
    assert(outIt == out.end());

    return out;
}

const int MetaTileTilesetData::TILESET_FORMAT_VERSION = 9;

std::vector<uint8_t>
MetaTileTilesetData::exportSnesData() const
{
    std::vector<uint8_t> tmBlock = convertTileset();

    std::vector<uint8_t> out = lz4HcCompress(tmBlock);

    std::vector<uint8_t> atData = animatedTileset.exportAnimatedTileset();

    out.insert(out.end(), atData.begin(), atData.end());

    return out;
}

}
}
