/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "combinesmalltiles.h"
#include <algorithm>
#include <climits>
#include <functional>
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
    unsigned firstTile = UINT_MAX;
    unsigned secondTile = UINT_MAX;
    std::vector<const MS::Frame*> frames;
};

static int scoreTiles(const std::vector<const MS::Frame*>& a, const std::vector<const MS::Frame*>& b)
{
    int score = 0;
    for (const auto& f : a) {
        int count = std::count(b.begin(), b.end(), f);
        if (count > 0) {
            score += 3 * count;
        }
        else {
            score -= 1;
        }
    }
    return score;
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

    for (unsigned i = 0; i < tileFrames.size(); i++) {
        if (not tileFrames.at(i).empty()) {
            smallTileGraph.emplace_back(i, std::move(tileFrames.at(i)));
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
        const TileGraphItem& mostPopular = *mostPopularIt;

        auto bestMatchIt = toProcess.end();
        {
            int bestScore = INT_MIN;
            for (auto it = mostPopularIt + 1; it != toProcess.end(); ++it) {
                const TileGraphItem& tgi = *it;
                int score = scoreTiles(tgi.frames, mostPopular.frames);

                if (score > bestScore) {
                    bestMatchIt = it;
                    bestScore = score;
                }
            }
        }

        assert(bestMatchIt != toProcess.end());
        const TileGraphItem& bestMatch = *bestMatchIt;

        output.emplace_back();
        auto& o = output.back();

        o.firstTile = mostPopular.tileId;
        o.secondTile = bestMatch.tileId;

        o.frames = mostPopular.frames;
        o.frames.insert(o.frames.end(), bestMatch.frames.begin(), bestMatch.frames.end());

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
        return a.frames.size() > b.frames.size();
    });

    while (!input.empty()) {
        auto mostPopular = input.begin();

        auto bestMatch = input.end();
        {
            int bestScore = INT_MIN;
            auto it = mostPopular;
            for (++it; it != input.end(); ++it) {
                int score = scoreTiles(mostPopular->frames, it->frames);

                if (score > bestScore) {
                    bestMatch = it;
                    bestScore = score;
                }
            }
        }

        assert(bestMatch != input.end());

        std::array<uint16_t, 4> combined;

        combined[0] = mostPopular->firstTile;
        combined[1] = mostPopular->secondTile;
        combined[2] = bestMatch->firstTile;
        combined[3] = bestMatch->secondTile;

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
    auto fp = CombineSmallTiles::firstPass(smallTileGraph);
    return CombineSmallTiles::secondPass(fp, frameSet.smallTileset.size());
}
}
}
}
