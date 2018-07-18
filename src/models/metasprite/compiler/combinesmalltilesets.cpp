/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "combinesmalltilesets.h"
#include <algorithm>
#include <climits>
#include <functional>
#include <list>

namespace UnTech {
namespace MetaSprite {
namespace Compiler {
namespace CombineSmallTilesets {

// This is a simple packing algorithm
// It just pairs off the most common small tiles,
// and then pairs them again.

namespace MS = UnTech::MetaSprite::MetaSprite;

// Graph of tileId => frames that use that tile.
// frames may be repeated if the same tile is used multiple times in the same frame (including flipped frames).
typedef std::vector<std::vector<const MetaSprite::Frame*>> TileGraph_t;

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

static TileGraph_t buildSmallTileGraph(const MetaSprite::FrameSet& frameSet,
                                       const std::vector<FrameListEntry>& frameEntries)
{
    TileGraph_t smallTileGraph(frameSet.smallTileset.size());

    for (const auto& fle : frameEntries) {
        if (fle.frame != nullptr) {
            for (const auto& obj : fle.frame->objects) {
                if (obj.size == ObjectSize::SMALL) {
                    smallTileGraph.at(obj.tileId).emplace_back(fle.frame);
                }
            }
        }
    }

    return smallTileGraph;
}

static std::list<FirstPassOutput> firstPass(const TileGraph_t& smallTileGraph)
{
    std::list<FirstPassOutput> output;

    std::vector<std::pair<unsigned, std::reference_wrapper<const std::vector<const MetaSprite::Frame*>>>> toProcess;
    for (unsigned i = 0; i < smallTileGraph.size(); i++) {
        if (!smallTileGraph.at(i).empty()) {
            toProcess.emplace_back(i, smallTileGraph.at(i));
        }
    }

    // sort toProcess by popularity
    std::stable_sort(toProcess.begin(), toProcess.end(),
                     [](const auto& a, const auto& b) {
                         return a.second.get().size() > b.second.get().size();
                     });

    while (!toProcess.empty()) {
        auto mostPopular = toProcess.begin();

        auto bestMatch = toProcess.end();
        {
            const std::vector<const MetaSprite::Frame*>& cmp(mostPopular->second);

            int bestScore = INT_MIN;
            auto it = toProcess.begin();
            for (++it; it != toProcess.end(); ++it) {
                int score = scoreTiles(it->second, cmp);

                if (score > bestScore) {
                    bestMatch = it;
                    bestScore = score;
                }
            }
        }

        output.emplace_back();
        auto& o = output.back();

        o.firstTile = mostPopular->first;
        o.frames = mostPopular->second.get();

        if (bestMatch != toProcess.end()) {
            o.secondTile = bestMatch->first;

            const auto& bmFrames = bestMatch->second.get();
            o.frames.insert(o.frames.end(), bmFrames.begin(), bmFrames.end());

            toProcess.erase(bestMatch);
        }

        toProcess.erase(mostPopular);
    }

    return output;
}

static SmallTileMap_t secondPass(std::list<FirstPassOutput> input,
                                 size_t nSmallTiles)
{
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

        std::array<uint16_t, 4> combined = INVALID_SMALL_TILES_ARRAY;

        combined[0] = mostPopular->firstTile;
        combined[1] = mostPopular->secondTile;

        if (bestMatch != input.end()) {
            combined[2] = bestMatch->firstTile;
            combined[3] = bestMatch->secondTile;

            input.erase(bestMatch);
        }
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
    auto smallTileGraph = CombineSmallTilesets::buildSmallTileGraph(frameSet, frameEntries);
    auto fp = CombineSmallTilesets::firstPass(smallTileGraph);
    return CombineSmallTilesets::secondPass(fp, smallTileGraph.size());
}
}
}
}
