#ifndef _UNTECH_MODELS_UTSI2UTMS_TILESETINSERTER_H_
#define _UNTECH_MODELS_UTSI2UTMS_TILESETINSERTER_H_

#include "../snes/tileset.h"
#include "../snes/tileset.hpp"
#include "../metasprite/frameobject.h"
#include <unordered_map>

namespace UnTech {
namespace Utsi2UtmsPrivate {

template <size_t ASIZE>
struct TileHash {
    size_t operator()(std::array<uint8_t, ASIZE> const& tile) const
    {
        constexpr unsigned BLOCK_SIZE = 4;
        constexpr unsigned LOOP_COUNT = (ASIZE - sizeof(size_t)) / BLOCK_SIZE;

        static_assert(sizeof(size_t) >= BLOCK_SIZE, "Bad assumption");
        static_assert(sizeof(size_t) % BLOCK_SIZE == 0, "Bad assumption");

        size_t seed = 0;
        const uint8_t* ptr = tile.data();

        for (unsigned i = 0; i < LOOP_COUNT; i++) {
            // Numbers from boost
            seed ^= (*(const size_t*)ptr) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            ptr += BLOCK_SIZE;
        }

        return seed;
    }
};

struct TilesetInserterOutput {
    unsigned tileId;
    bool hFlip;
    bool vFlip;

    void apply(MetaSprite::FrameObject& fo)
    {
        fo.setTileId(tileId);
        fo.setHFlip(hFlip);
        fo.setVFlip(vFlip);
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

    const TilesetInserterOutput getOrInsert(const typename T::tileData_t& tile)
    {
        auto it = _map.find(tile);

        if (it != _map.end()) {
            return it->second;
        }
        else {
            return insertNewTile(tile);
        }
    }

    const typename T::tileData_t getTile(const TilesetInserterOutput& tio) const
    {
        return _tileset.tile(tio.tileId, tio.hFlip, tio.vFlip);
    }

    const std::pair<TilesetInserterOutput, bool>
    processOverlappedTile(const typename T::tileData_t& underTile,
                          const typename std::array<bool, T::TILE_DATA_SIZE>& overlaps)
    {
        unsigned bestScore = 0;
        TilesetInserterOutput ret = { 0, false, false };

        for (auto it : _map) {
            const typename T::tileData_t& other = it.first;
            unsigned score = 0;
            bool found = true;

            for (unsigned i = 0; i < T::TILE_DATA_SIZE; i++) {
                if (underTile[i] == other[i]) {
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
    TilesetInserterOutput insertNewTile(const typename T::tileData_t& tile)
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

        _map.insert({ _tileset.tile(tileId),
                      { tileId, false, false } });

        _map.insert({ _tileset.tileHFlip(tileId),
                      { tileId, false, true } });

        _map.insert({ _tileset.tileVFlip(tileId),
                      { tileId, true, false } });

        _map.insert({ _tileset.tileHVFlip(tileId),
                      { tileId, true, true } });
    }

private:
    T& _tileset;

    std::unordered_map<typename T::tileData_t, TilesetInserterOutput,
                       TileHash<T::TILE_DATA_SIZE>> _map;
};
}
}

#endif
