/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "tile.h"
#include "models/common/iterators.h"
#include <unordered_map>

namespace UnTech::Snes {

struct TilesetInserterOutput {
    unsigned tileId;
    bool hFlip;
    bool vFlip;

    bool operator==(const TilesetInserterOutput&) const = default;
};

template <size_t TS>
class TilesetInserter {
public:
    using TileT = Tile<TS>;
    using TilesetT = std::vector<TileT>;

private:
    TilesetT& _tileset;

    std::unordered_map<TileT, TilesetInserterOutput> _map;

public:
    TilesetInserter(TilesetT& tileset)
        : _tileset(tileset)
        , _map()
    {
        for (const auto t : range(tileset.size())) {
            addToMap(t);
        }
    }

    const TilesetInserterOutput getOrInsert(const TileT& tile)
    {
        auto it = _map.find(tile);

        if (it != _map.end()) {
            return it->second;
        }
        else {
            return insertNewTile(tile);
        }
    }

    const TileT getTile(const TilesetInserterOutput& tio) const
    {
        return _tileset.at(tio.tileId).flip(tio.hFlip, tio.vFlip);
    }

    const std::pair<TilesetInserterOutput, bool>
    processOverlappedTile(const TileT& underTile,
                          const std::array<bool, TileT::TILE_ARRAY_SIZE>& overlaps)
    {
        unsigned bestScore = 0;
        TilesetInserterOutput ret = { 0, false, false };

        const auto& underTileData = underTile.data();

        // Using _tileset instead of _map to ensure the tiles are tested in a deterministic order.
        for (const auto [i, tile] : const_enumerate(_tileset)) {
            const auto tileId = i;
            auto testTile = [&](const TileT& toTest, const bool hFlip, const bool vFlip) {
                const auto& toTestData = toTest.data();

                unsigned score = 0;
                bool found = true;

                for (const auto i : range(TileT::TILE_ARRAY_SIZE)) {
                    if (underTileData[i] == toTestData[i]) {
                        score++;
                    }
                    else if (overlaps[i] == false) {
                        found = false;
                        break;
                    }
                }

                if (found && score > bestScore) {
                    bestScore = score;
                    ret = { unsigned(tileId), hFlip, vFlip };
                }
            };

            testTile(tile, false, false);
            testTile(tile.hFlip(), true, false);
            testTile(tile.vFlip(), false, true);
            testTile(tile.hvFlip(), true, true);
        }

        if (bestScore != 0) {
            return { ret, true };
        }
        else {
            return { insertNewTile(underTile), false };
        }
    }

private:
    TilesetInserterOutput insertNewTile(const TileT& tile)
    {
        unsigned tileId = _tileset.size();
        _tileset.push_back(tile);

        addToMap(tileId);

        return { tileId, false, false };
    }

    void addToMap(unsigned tileId)
    {
        // unordered_map will ignore insert if tile pattern already exists
        // Thus symmetrical tiles will prefer the unflipped tile.

        const auto& tile = _tileset.at(tileId);

        _map.insert({ tile, { tileId, false, false } });
        _map.insert({ tile.hFlip(), { tileId, true, false } });
        _map.insert({ tile.vFlip(), { tileId, false, true } });
        _map.insert({ tile.hvFlip(), { tileId, true, true } });
    }
};

typedef TilesetInserter<8> TilesetInserter8px;
typedef TilesetInserter<16> TilesetInserter16px;

}
