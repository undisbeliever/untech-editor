/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "tileset.h"
#include "tilesetinserter.h"
#include <algorithm>
#include <unordered_map>
#include <vector>

namespace UnTech {
namespace Snes {

template <class T>
class AnimatedTilesetInserter {
    using TilesetT = T;
    using TileT = typename T::TileT;

public:
    AnimatedTilesetInserter(std::vector<TilesetT>& tilesets)
        : _tilesets(tilesets)
        , _map()
    {
        assert(nFrames() > 0);

        unsigned nTiles = tilesets.front().size();

        for (unsigned f = 0; f < nFrames(); f++) {
            assert(tilesets.at(f).size() == nTiles);
        }

        std::vector<TileT> tiles(tilesets.size());

        for (unsigned t = 0; t < tilesets.front().size(); t++) {
            for (unsigned f = 0; f < nFrames(); f++) {
                tiles.at(f) = tilesets.at(f).tile(t);
            }
            addToMap(tiles, t);
        }
    }

    unsigned nFrames() const { return _tilesets.size(); }

    const TilesetInserterOutput getOrInsert(const std::vector<TileT>& tiles)
    {
        assert(tiles.size() == nFrames());

        auto it = _map.find(tiles);

        if (it != _map.end()) {
            return it->second;
        }
        else {
            return insertNewTile(tiles);
        }
    }

private:
    TilesetInserterOutput insertNewTile(const std::vector<TileT>& tiles)
    {
        assert(tiles.size() == _tilesets.size());

        unsigned tileId = _tilesets.front().size();

        for (unsigned f = 0; f < _tilesets.size(); f++) {
            _tilesets.at(f).addTile(tiles.at(f));
        }

        addToMap(tiles, tileId);

        return { tileId, false, false };
    }

    void addToMap(const std::vector<TileT>& tiles, unsigned tileId)
    {
        assert(tiles.size() == nFrames());

        std::vector<TileT> hFliped(tiles.size());
        std::transform(tiles.begin(), tiles.end(), hFliped.begin(),
                       [](const auto& t) { return t.hFlip(); });

        std::vector<TileT> vFliped(tiles.size());
        std::transform(tiles.begin(), tiles.end(), vFliped.begin(),
                       [](const auto& t) { return t.vFlip(); });

        std::vector<TileT> hvFliped(tiles.size());
        std::transform(tiles.begin(), tiles.end(), hvFliped.begin(),
                       [](const auto& t) { return t.hvFlip(); });

        // unordered_map will ignore insert if tile pattern already exists
        // Thus symmetrical tiles will prefer the unflipped tile.

        _map.insert({ tiles, { tileId, false, false } });
        _map.insert({ hFliped, { tileId, true, false } });
        _map.insert({ vFliped, { tileId, false, true } });
        _map.insert({ hvFliped, { tileId, true, true } });
    }

private:
    std::vector<TilesetT>& _tilesets;
    std::unordered_map<std::vector<TileT>, TilesetInserterOutput> _map;
};
}
}
