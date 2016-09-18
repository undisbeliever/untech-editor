#pragma once

#include "../metasprite.h"
#include "models/snes/tileset.h"
#include "models/snes/tileset.hpp"
#include <unordered_map>

namespace UnTech {
namespace MetaSprite {
namespace Utsi2UtmsPrivate {

struct TilesetInserterOutput {
    unsigned tileId;
    bool hFlip;
    bool vFlip;

    void apply(MetaSprite::FrameObject& fo)
    {
        fo.tileId = tileId;
        fo.hFlip = hFlip;
        fo.vFlip = vFlip;
    }

    bool operator==(const TilesetInserterOutput& o) const
    {
        return tileId == o.tileId
               && hFlip == o.hFlip
               && vFlip == o.vFlip;
    }
};

template <class T>
class TilesetInserter {
public:
    TilesetInserter(T& tileset)
        : _tileset(tileset)
        , _map()
    {
        for (unsigned t = 0; t < tileset.size(); t++) {
            addToMap(t);
        }
    }

    const TilesetInserterOutput getOrInsert(const typename T::tile_t& tile)
    {
        auto it = _map.find(tile);

        if (it != _map.end()) {
            return it->second;
        }
        else {
            return insertNewTile(tile);
        }
    }

    const typename T::tile_t getTile(const TilesetInserterOutput& tio) const
    {
        return _tileset.tile(tio.tileId).flip(tio.hFlip, tio.vFlip);
    }

    const std::pair<TilesetInserterOutput, bool>
    processOverlappedTile(const typename T::tile_t& underTile,
                          const typename std::array<bool, T::tile_t::TILE_ARRAY_SIZE>& overlaps)
    {
        unsigned bestScore = 0;
        TilesetInserterOutput ret = { 0, false, false };

        const uint8_t* underTileData = underTile.rawData();

        for (const auto& it : _map) {
            const uint8_t* other = it.first.rawData();

            unsigned score = 0;
            bool found = true;

            for (unsigned i = 0; i < T::tile_t::TILE_ARRAY_SIZE; i++) {
                if (underTileData[i] == other[i]) {
                    score++;
                }
                else if (overlaps[i] == false) {
                    found = false;
                    break;
                }
            }

            if (found && score > bestScore) {
                bestScore = score;
                ret = it.second;
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
    TilesetInserterOutput insertNewTile(const typename T::tile_t& tile)
    {
        unsigned tileId = _tileset.size();
        _tileset.addTile(tile);

        addToMap(tileId);

        return { tileId, false, false };
    }

    void addToMap(unsigned tileId)
    {
        // unordered_map will ignore insert if tile pattern already exists
        // Thus symmetrical tiles will prefer the unflipped tile.

        const auto& tile = _tileset.tile(tileId);

        _map.insert({ tile,
                      { tileId, false, false } });

        _map.insert({ tile.hFlip(),
                      { tileId, false, true } });

        _map.insert({ tile.vFlip(),
                      { tileId, true, false } });

        _map.insert({ tile.hvFlip(),
                      { tileId, true, true } });
    }

private:
    T& _tileset;

    std::unordered_map<typename T::tile_t, TilesetInserterOutput> _map;
};
}
}
}
