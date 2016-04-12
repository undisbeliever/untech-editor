#ifndef _UNTECH_MODELS_UTSI2UTMS_TILESETINSERTER_H_
#define _UNTECH_MODELS_UTSI2UTMS_TILESETINSERTER_H_

#include "../snes/tileset.h"
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
            unsigned t = _tileset.size();
            _tileset.addTile();
            _tileset.tile(t) = tile;

            addToMap(tile, t);

            return { t, false, false };
        }
    }

private:
    void addToMap(const typename T::tileData_t& tile, unsigned tileId)
    {
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
