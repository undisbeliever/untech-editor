/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "combinesmalltilesets.h"
#include "models/common/vectorset.h"
#include <algorithm>
#include <climits>
#include <list>

namespace UnTech {
namespace MetaSprite {
namespace Compiler {
namespace CombineSmallTilesets {

// This is a simple packing algorithm
// It just pairs off the most common small tiles,
// and then pairs them again.

namespace MS = UnTech::MetaSprite::MetaSprite;

struct FirstPassOutput {
    unsigned firstTile = UINT_MAX;
    unsigned secondTile = UINT_MAX;
    vectorset<const MS::Frame*> frames;
};

int scoreTiles(const std::vector<const MS::Frame*>& a, const vectorset<const MS::Frame*>& b)
{
    int score = 0;
    for (const auto& f : a) {
        if (b.contains(f)) {
            score += 3;
        }
        else {
            score -= 1;
        }
    }
    return score;
}

int scoreTiles(const vectorset<const MS::Frame*>& a, const vectorset<const MS::Frame*>& b)
{
    int score = 0;
    for (const auto& f : a) {
        if (b.contains(f)) {
            score += 3;
        }
        else {
            score -= 1;
        }
    }
    return score;
}

inline std::list<FirstPassOutput> firstPass(const TileGraph_t& smallTileGraph)
{
    std::list<FirstPassOutput> output;

    std::list<std::pair<unsigned, const std::vector<const MetaSprite::Frame*>&>> toProcess;
    for (unsigned i = 0; i < smallTileGraph.size(); i++) {
        if (!smallTileGraph.at(i).empty()) {
            toProcess.emplace_back(i, smallTileGraph.at(i));
        }
    }

    // sort the list by popularity
    toProcess.sort([](const auto& a, const auto& b) {
        return a.second.size() > b.second.size();
    });

    while (!toProcess.empty()) {
        auto mostPopular = toProcess.begin();

        auto bestMatch = toProcess.end();
        {
            const vectorset<const MetaSprite::Frame*> cmp(mostPopular->second);

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
        for (const auto& f : mostPopular->second) {
            o.frames.insert(f);
        }

        if (bestMatch != toProcess.end()) {
            o.secondTile = bestMatch->first;

            for (const auto& f : bestMatch->second) {
                o.frames.insert(f);
            }

            toProcess.erase(bestMatch);
        }

        toProcess.erase(mostPopular);
    }

    return output;
}

inline SmallTileMap_t secondPass(std::list<FirstPassOutput> input,
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

SmallTileMap_t combineSmallTilesets(const TileGraph_t& smallTileGraph)
{
    auto fp = CombineSmallTilesets::firstPass(smallTileGraph);
    return CombineSmallTilesets::secondPass(fp, smallTileGraph.size());
}
}
}
}
