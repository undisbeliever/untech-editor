/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tilesetlayout.h"
#include "combinesmalltiles.h"
#include "tilesetinserter.h"
#include "tilesettype.hpp"
#include "models/common/errorlist.h"
#include "models/common/iterators.h"
#include "models/common/vectorset.h"
#include "models/metasprite/errorlisthelpers.h"

namespace UnTech::MetaSprite::Compiler {

namespace MS = UnTech::MetaSprite::MetaSprite;

static void addFrameToTileset(vectorset<Tile16>& tiles,
                              const MS::Frame& frame,
                              const SmallTileMap_t& smallTileMap)
{
    for (const auto& obj : frame.objects) {
        Tile16 t;
        if (obj.size == ObjectSize::LARGE) {
            t.largeTileId = obj.tileId;
        }
        else {
            t.smallTileIds = smallTileMap.at(obj.tileId);
        }
        tiles.insert(t);
    }
}

static vectorset<Tile16> fixedTilesetData(const std::vector<ExportIndex>& frames,
                                          const MS::FrameSet& frameSet,
                                          const SmallTileMap_t& smallTileMap)
{
    vectorset<Tile16> tiles;

    for (const auto& entry : frames) {
        addFrameToTileset(tiles, frameSet.frames.at(entry.fsIndex), smallTileMap);
    }

    return tiles;
}

struct DynamicTileset {
    vectorset<Tile16> tiles;
    std::vector<unsigned> frameIds;

    DynamicTileset(vectorset<Tile16>&& tiles, unsigned frameId)
        : tiles(std::move(tiles))
        , frameIds({ frameId })
    {
    }
};

static std::vector<DynamicTileset> tilesForEachFrame(const std::vector<ExportIndex>& frameEntries,
                                                     const MS::FrameSet& frameSet,
                                                     const SmallTileMap_t& smallTileMap)
{
    std::vector<DynamicTileset> ret;

    for (auto [frameId, entry] : const_enumerate(frameEntries)) {
        const auto frame = frameSet.frames.at(entry.fsIndex);

        vectorset<Tile16> tileset;
        addFrameToTileset(tileset, frame, smallTileMap);

        auto it = std::find_if(ret.begin(), ret.end(),
                               [&](auto& i) { return i.tiles == tileset; });

        if (it == ret.end()) {
            ret.emplace_back(std::move(tileset), frameId);
        }
        else {
            // tileset already exists
            it->frameIds.push_back(frameId);
        }
    }

    return ret;
}

static std::vector<std::pair<Tile16, unsigned>> countTileUsage(const std::vector<DynamicTileset>& ftVector)
{
    std::vector<std::pair<Tile16, unsigned>> ret;
    ret.reserve(128);

    for (const auto& ft : ftVector) {
        for (const Tile16& ftTile : ft.tiles) {
            auto it = std::find_if(ret.begin(), ret.end(),
                                   [&](const auto& i) { return i.first == ftTile; });
            if (it == ret.end()) {
                ret.emplace_back(ftTile, ft.frameIds.size());
            }
            else {
                it->second += ft.frameIds.size();
            }
        }
    }

    return ret;
}

static vectorset<Tile16> calculateStaticTiles(const std::vector<DynamicTileset>& ftVector,
                                              const TilesetType tilesetType)
{
    const unsigned tilesetType_nTiles = numberOfTilesetTiles(tilesetType);

    auto popularTiles = countTileUsage(ftVector);

    std::stable_sort(popularTiles.begin(), popularTiles.end(),
                     [](auto& l, auto& r) { return l.second > r.second; });

    if (popularTiles.size() > tilesetType_nTiles - 1) {
        popularTiles.resize(tilesetType_nTiles - 1);
    }

    while (popularTiles.size() > 0) {
        unsigned maxDynamicTiles = 0;

        for (const auto& ft : ftVector) {
            unsigned nDynamicTiles = ft.tiles.size();

            for (const auto& tc : popularTiles) {
                if (ft.tiles.contains(tc.first)) {
                    nDynamicTiles--;
                }
            }
            if (nDynamicTiles > maxDynamicTiles) {
                maxDynamicTiles = nDynamicTiles;
            }
        }

        if (maxDynamicTiles + popularTiles.size() <= tilesetType_nTiles) {
            break;
        }
        popularTiles.resize(popularTiles.size() - 1);
    }

    vectorset<Tile16> ret;
    ret.reserve(popularTiles.size());
    for (const auto& tc : popularTiles) {
        ret.insert(tc.first);
    }
    return ret;
}

static std::vector<Tile16> tile_difference(const vectorset<Tile16>& tiles,
                                           const vectorset<Tile16>& staticTiles)
{
    std::vector<Tile16> ret;
    ret.reserve(tiles.size());

    std::copy_if(tiles.begin(), tiles.end(),
                 std::back_inserter(ret),
                 [&](const Tile16& t) { return staticTiles.contains(t) == false; });

    return ret;
}

TilesetLayout layoutTiles(const MS::FrameSet& frameSet,
                          const std::vector<ExportIndex>& exportFrames,
                          ErrorList& errorList)
{
    const TilesetType tilesetType = frameSet.tilesetType;
    const unsigned tilesetType_nTiles = numberOfTilesetTiles(tilesetType);
    const bool tilesetType_isFixed = isFixedTilesetType(tilesetType);

    const auto smallTileMap = buildSmallTileMap(frameSet, exportFrames);
    const auto tiles = fixedTilesetData(exportFrames, frameSet, smallTileMap);

    TilesetLayout ret;
    ret.tilesetType = frameSet.tilesetType;
    ret.frameTilesets.resize(exportFrames.size(), -1);

    if (tiles.size() < tilesetType_nTiles || tilesetType_isFixed) {
        // Fixed tileset
        if (not tilesetType_isFixed) {
            errorList.addWarningString("Tileset can be fixed, making it so.");
        }

        if (tiles.size() <= tilesetType_nTiles) {
            const TilesetType smallestType = smallestFixedTilesetType(tiles.size());
            if (numberOfTilesetTiles(smallestType) != tilesetType_nTiles) {
                errorList.addWarningString("TilesetType shrunk to ", ttString(smallestType));
            }
            ret.tilesetType = smallestType;
            ret.staticTiles = std::move(tiles);
        }
        else {
            errorList.addErrorString("Unable to fit ", tiles.size(), " Tile16 tiles inside a ", ttString(tilesetType));
        }
    }
    else {
        // Dynamic tileset
        const auto frameTiles = tilesForEachFrame(exportFrames, frameSet, smallTileMap);
        ret.staticTiles = calculateStaticTiles(frameTiles, tilesetType);

        for (const auto& ft : frameTiles) {
            auto dynamicTiles = tile_difference(ft.tiles, ret.staticTiles);
            if (dynamicTiles.empty() == false) {
                unsigned nTiles = dynamicTiles.size() + ret.staticTiles.size();

                if (nTiles <= tilesetType_nTiles) {
                    int tilesetId = ret.dynamicTiles.size();
                    ret.dynamicTiles.push_back(std::move(dynamicTiles));
                    for (unsigned frameId : ft.frameIds) {
                        ret.frameTilesets.at(frameId) = tilesetId;
                    }
                }
                else {
                    for (unsigned frameId : ft.frameIds) {
                        const unsigned frameIndex = exportFrames.at(frameId).fsIndex;
                        errorList.addError(frameError(
                            frameSet.frames.at(frameIndex), frameIndex,
                            "Too many tiles in frame (", nTiles, ")", dynamicTiles.size(), " ", ret.staticTiles.size()));
                    }
                }
            }
        }
    }

    return ret;
}

}
