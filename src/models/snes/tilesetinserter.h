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

    bool operator==(const TilesetInserterOutput& o) const
    {
        return tileId == o.tileId
               && hFlip == o.hFlip
               && vFlip == o.vFlip;
    }
};

template <size_t TS>
class TilesetInserter {
    using TileT = Tile<TS>;
    using TilesetT = std::vector<TileT>;

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

        const uint8_t* underTileData = underTile.rawData();

        // Using _tileset instead of _map to ensure the tiles are tested in a deterministic order.
        for (const auto [tileId, tile] : const_enumerate(_tileset)) {
            for (const auto flip : range(4)) {
                bool hFlip = flip & 1;
                bool vFlip = flip & 2;

                const TileT toTest = tileId == 0 ? tile
                                                 : tile.flip(hFlip, vFlip);
                const uint8_t* toTestData = toTest.rawData();

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
            }
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

private:
    TilesetT& _tileset;

    std::unordered_map<TileT, TilesetInserterOutput> _map;
};

typedef TilesetInserter<8> TilesetInserter8px;
typedef TilesetInserter<16> TilesetInserter16px;

}
