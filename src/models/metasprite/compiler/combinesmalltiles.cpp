/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "combinesmalltiles.h"
#include "tilesetlayout.h"
#include "models/common/iterators.h"
#include <algorithm>
#include <climits>
#include <list>

namespace UnTech {
namespace MetaSprite {
namespace Compiler {
namespace CombineSmallTiles {

// This is a simple packing algorithm
// It just pairs off the most common small tiles,
// and then pairs them again.

namespace MS = UnTech::MetaSprite::MetaSprite;

struct TileGraphItem {
    // Graph of tileId => frames that use that tile.
    // frames may be repeated if the same tile is used multiple times in the same frame (including flipped frames).

    const unsigned tileId;
    const std::vector<const MetaSprite::Frame*> frames;

    TileGraphItem()
        : tileId(INVALID_SMALL_TILE)
        , frames()
    {
    }
    TileGraphItem(unsigned t, std::vector<const MetaSprite::Frame*>&& f)
        : tileId(t)
        , frames(std::move(f))
    {
    }
};

struct FirstPassOutput {
    const TileGraphItem& firstTile;
    const TileGraphItem& secondTile;

    FirstPassOutput(const TileGraphItem& first, const TileGraphItem& second)
        : firstTile(first)
        , secondTile(second)
    {
    }

    size_t nFrames() const { return firstTile.frames.size() + secondTile.frames.size(); }
};

static int scoreTilesVec(const std::vector<const MetaSprite::Frame*>& a, const std::vector<const MetaSprite::Frame*>& b)
{
    int score = 0;
    for (const auto& f : a) {
        int count = std::count(b.begin(), b.end(), f);
        if (count > 0) {
            score += count;
        }
        else {
            score -= 3;
        }
    }
    return score;
}

static int scoreTiles(const TileGraphItem& a, const TileGraphItem& b)
{
    return scoreTilesVec(a.frames, b.frames)
           + scoreTilesVec(b.frames, a.frames);
}

static int scoreTiles(const FirstPassOutput& a, const FirstPassOutput& b)
{
    return scoreTiles(a.firstTile, b.firstTile)
           + scoreTiles(a.secondTile, b.firstTile)
           + scoreTiles(b.firstTile, a.secondTile)
           + scoreTiles(a.secondTile, b.secondTile);
}

// To improve packing the size of the output is always a multiple of four (4).
static std::vector<TileGraphItem> buildSmallTileGraph(const MetaSprite::FrameSet& frameSet,
                                                      const std::vector<FrameListEntry>& frameEntries)
{
    std::vector<std::vector<const MetaSprite::Frame*>> tileFrames(frameSet.smallTileset.size());
    for (const auto& fle : frameEntries) {
        if (fle.frame != nullptr) {
            for (const auto& obj : fle.frame->objects) {
                if (obj.size == ObjectSize::SMALL) {
                    tileFrames.at(obj.tileId).emplace_back(fle.frame);
                }
            }
        }
    }

    std::vector<TileGraphItem> smallTileGraph;
    smallTileGraph.reserve(int((frameSet.smallTileset.size() + 3) / 4) * 4);

    for (auto [i, tf] : enumerate(tileFrames)) {
        if (not tf.empty()) {
            smallTileGraph.emplace_back(i, std::move(tf));
        }
    }

    while (smallTileGraph.size() % 4 != 0) {
        smallTileGraph.emplace_back();
    }

    return smallTileGraph;
}

static std::list<FirstPassOutput> firstPass(const std::vector<TileGraphItem>& smallTileGraph)
{
    assert(!smallTileGraph.empty() && smallTileGraph.size() % 4 == 0);

    std::list<FirstPassOutput> output;

    std::vector<std::reference_wrapper<const TileGraphItem>> toProcess;
    toProcess.reserve(smallTileGraph.size());
    for (const TileGraphItem& it : smallTileGraph) {
        toProcess.emplace_back(it);
    }

    // sort toProcess by popularity
    std::stable_sort(toProcess.begin(), toProcess.end(),
                     [](const TileGraphItem& a, const TileGraphItem& b) {
                         return a.frames.size() > b.frames.size();
                     });

    while (!toProcess.empty()) {
        assert(toProcess.size() % 2 == 0);

        auto mostPopularIt = toProcess.begin();

        auto bestMatchIt = toProcess.end();
        {
            int bestScore = INT_MIN;
            for (auto it = mostPopularIt + 1; it != toProcess.end(); ++it) {
                int score = scoreTiles(*it, *mostPopularIt);

                if (score > bestScore) {
                    bestMatchIt = it;
                    bestScore = score;
                }
            }
        }
        assert(bestMatchIt != toProcess.end());

        output.emplace_back(*mostPopularIt, *bestMatchIt);

        toProcess.erase(bestMatchIt);
        toProcess.erase(mostPopularIt);
    }

    return output;
}

static SmallTileMap_t secondPass(std::list<FirstPassOutput>& input,
                                 const size_t nSmallTiles)
{
    assert(!input.empty() && input.size() % 2 == 0);

    SmallTileMap_t output(nSmallTiles, INVALID_SMALL_TILES_ARRAY);

    // sort the input by popularity
    // should be mostly sorted anyways
    input.sort([](const FirstPassOutput& a, const FirstPassOutput& b) {
        return a.nFrames() > b.nFrames();
    });

    while (!input.empty()) {
        auto mostPopular = input.begin();

        auto bestMatch = input.end();
        {
            int bestScore = INT_MIN;
            auto it = mostPopular;
            for (++it; it != input.end(); ++it) {
                int score = scoreTiles(*mostPopular, *it);

                if (score > bestScore) {
                    bestMatch = it;
                    bestScore = score;
                }
            }
        }

        assert(bestMatch != input.end());

        std::array<uint16_t, 4> combined;

        combined[0] = mostPopular->firstTile.tileId;
        combined[1] = mostPopular->secondTile.tileId;
        combined[2] = bestMatch->firstTile.tileId;
        combined[3] = bestMatch->secondTile.tileId;

        input.erase(bestMatch);
        input.erase(mostPopular);

        for (auto tId : combined) {
            if (tId != INVALID_SMALL_TILE) {
                output.at(tId) = combined;
            }
        }
    }

    return output;
}
}

SmallTileMap_t buildSmallTileMap(const MetaSprite::FrameSet& frameSet,
                                 const std::vector<FrameListEntry>& frameEntries)
{
    if (frameSet.smallTileset.empty()) {
        return SmallTileMap_t();
    }

    auto smallTileGraph = CombineSmallTiles::buildSmallTileGraph(frameSet, frameEntries);
    if (smallTileGraph.empty()) {
        return SmallTileMap_t();
    }
    auto fp = CombineSmallTiles::firstPass(smallTileGraph);
    return CombineSmallTiles::secondPass(fp, frameSet.smallTileset.size());
}
}
}
}
