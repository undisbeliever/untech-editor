#ifndef _UNTECH_MODELS_UTSI2UTMS_TILESETINSERTER_H_
#define _UNTECH_MODELS_UTSI2UTMS_TILESETINSERTER_H_

#include "../snes/tileset.h"
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
            addToMap(tileset.tile(t), t);
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
        static typename T::tileData_t zeroData = {};

        // ::TODO optimize::
        // ::: - get tile from the tileset and preform a flip::

        for (auto it : _map) {
            if (it.second == tio) {
                return it.first;
            }
        }
        return zeroData;
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
        _tileset.addTile();
        _tileset.tile(tileId) = tile;

        addToMap(tile, tileId);

        return { tileId, false, false };
    }

    void addToMap(const typename T::tileData_t& tile, unsigned tileId)
    {
        // ::TODO check how symmetrical tiles are handled::

        constexpr unsigned TILE_SIZE = T::TILE_SIZE;

        const uint8_t(*pixelData)[TILE_SIZE] = (uint8_t(*)[TILE_SIZE])tile.data();

        _map.insert({ tile, { tileId, false, false } });

        // hflip
        typename T::tileData_t hFlip;
        uint8_t(*hData)[TILE_SIZE] = (uint8_t(*)[TILE_SIZE])hFlip.data();

        for (unsigned y = 0; y < TILE_SIZE; y++) {
            for (unsigned x = 0; x < TILE_SIZE; x++) {
                hData[y][x] = pixelData[y][TILE_SIZE - x - 1];
            }
        }
        _map.insert({ hFlip, { tileId, false, true } });

        // vflip
        typename T::tileData_t vFlip;
        uint8_t(*vData)[TILE_SIZE] = (uint8_t(*)[TILE_SIZE])hFlip.data();

        for (unsigned y = 0; y < TILE_SIZE; y++) {
            for (unsigned x = 0; x < TILE_SIZE; x++) {
                vData[y][x] = pixelData[TILE_SIZE - y - 1][x];
            }
        }
        _map.insert({ vFlip, { tileId, false, true } });

        // hvflip
        typename T::tileData_t hvFlip;
        uint8_t(*hvData)[TILE_SIZE] = (uint8_t(*)[TILE_SIZE])hFlip.data();
        for (unsigned y = 0; y < TILE_SIZE; y++) {
            for (unsigned x = 0; x < TILE_SIZE; x++) {
                hvData[y][x] = pixelData[TILE_SIZE - y - 1][TILE_SIZE - x - 1];
            }
        }
        _map.insert({ hvFlip, { tileId, true, true } });
    }

private:
    T& _tileset;

    std::unordered_map<typename T::tileData_t, TilesetInserterOutput,
                       TileHash<T::TILE_DATA_SIZE>> _map;
};
}
}

#endif
