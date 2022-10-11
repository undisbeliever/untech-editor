/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "tile.h"
#include "tilesetinserter.h"
#include <algorithm>
#include <unordered_map>
#include <vector>

namespace UnTech::Snes {

template <unsigned TILE_SIZE>
class AnimatedTilesetInserter {
    using TileT = Tile<TILE_SIZE>;
    using TilesetT = std::vector<TileT>;

private:
    std::vector<TilesetT>& _tilesets;
    std::unordered_map<std::vector<TileT>, TilesetInserterOutput> _map;

public:
    AnimatedTilesetInserter(std::vector<TilesetT>& tilesets)
        : _tilesets(tilesets)
        , _map()
    {
        assert(nFrames() > 0);

        unsigned nTiles = tilesets.front().size();

        for (const auto& tileset : tilesets) {
            assert(tileset.size() == nTiles);
        }

        std::vector<TileT> tiles(tilesets.size());

        for (const auto t : range(tilesets.front().size())) {
            for (const auto f : range(nFrames())) {
                tiles.at(f) = tilesets.at(f).at(t);
            }
            addToMap(tiles, t);
        }
    }

    [[nodiscard]] unsigned nFrames() const { return _tilesets.size(); }

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

        for (const auto f : range(_tilesets.size())) {
            _tilesets.at(f).push_back(tiles.at(f));
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
};

}
