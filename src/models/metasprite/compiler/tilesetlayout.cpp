/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tilesetlayout.h"
#include "combinesmalltiles.h"
#include "tilesetinserter.h"
#include "models/common/errorlist.h"
#include "models/common/vectorset.h"
#include "models/metasprite/errorlisthelpers.h"

namespace MS = UnTech::MetaSprite::MetaSprite;

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

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

static vectorset<Tile16> fixedTilesetData(const std::vector<FrameListEntry>& frames,
                                          const SmallTileMap_t& smallTileMap)
{
    vectorset<Tile16> tiles;

    for (const auto& entry : frames) {
        addFrameToTileset(tiles, *entry.frame, smallTileMap);
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

static std::vector<DynamicTileset> tilesForEachFrame(const std::vector<FrameListEntry>& frameEntries,
                                                     const SmallTileMap_t& smallTileMap)
{
    std::vector<DynamicTileset> ret;

    for (unsigned frameId = 0; frameId < frameEntries.size(); frameId++) {
        const auto& entry = frameEntries.at(frameId);

        vectorset<Tile16> tileset;
        addFrameToTileset(tileset, *entry.frame, smallTileMap);

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
    auto popularTiles = countTileUsage(ftVector);

    std::stable_sort(popularTiles.begin(), popularTiles.end(),
                     [](auto& l, auto& r) { return l.second > r.second; });

    if (popularTiles.size() > tilesetType.nTiles() - 1) {
        popularTiles.resize(tilesetType.nTiles() - 1);
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

        if (maxDynamicTiles + popularTiles.size() <= tilesetType.nTiles()) {
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
                          const std::vector<FrameListEntry>& exportFrames,
                          ErrorList& errorList)
{
    TilesetLayout ret;

    const TilesetType tilesetType = frameSet.tilesetType;

    auto smallTileMap = buildSmallTileMap(frameSet, exportFrames);

    auto tiles = fixedTilesetData(exportFrames, smallTileMap);

    if (tiles.size() < tilesetType.nTiles() || tilesetType.isFixed()) {
        // Fixed tileset
        if (tilesetType.isFixed() == false) {
            errorList.addWarningString("Tileset can be fixed, making it so.");
        }

        if (tiles.size() <= tilesetType.nTiles()) {
            TilesetType smallestType = TilesetType::smallestFixedTileset(tiles.size());
            if (smallestType.nTiles() != tilesetType.nTiles()) {
                errorList.addWarningString("TilesetType shrunk to ", smallestType.string());
            }
            ret.tilesetType = smallestType;
            ret.staticTiles = std::move(tiles);

            ret.tilesetType = tilesetType;

            ret.frameTilesets.resize(exportFrames.size(), -1);
        }
        else {
            errorList.addErrorString("Unable to fit ", tiles.size(), " Tile16 tiles inside a ", tilesetType.string());
        }
    }
    else {
        // Dynamic tileset
        const auto frameTiles = tilesForEachFrame(exportFrames, smallTileMap);
        ret.staticTiles = calculateStaticTiles(frameTiles, tilesetType);
        ret.tilesetType = tilesetType;

        ret.frameTilesets.resize(exportFrames.size(), -1);
        for (const auto& ft : frameTiles) {
            auto dynamicTiles = tile_difference(ft.tiles, ret.staticTiles);
            if (dynamicTiles.empty() == false) {
                unsigned nTiles = dynamicTiles.size() + ret.staticTiles.size();

                if (nTiles <= tilesetType.nTiles()) {
                    int tilesetId = ret.dynamicTiles.size();
                    ret.dynamicTiles.push_back(std::move(dynamicTiles));
                    for (unsigned frameId : ft.frameIds) {
                        ret.frameTilesets.at(frameId) = tilesetId;
                    }
                }
                else {
                    for (unsigned frameId : ft.frameIds) {
                        errorList.addError(frameError(
                            *exportFrames.at(frameId).frame,
                            "Too many tiles in frame (", nTiles, ")", dynamicTiles.size(), " ", ret.staticTiles.size()));
                    }
                }
            }
        }
    }

    return ret;
}

}
}
}
