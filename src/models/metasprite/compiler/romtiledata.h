/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "romdata.h"
#include "models/project/memorymap.h"
#include "models/snes/tileset.h"
#include <cassert>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

// Tile data is stored in last ROM Bank first.

class RomTileData {
public:
    constexpr static unsigned SNES_TILE16_SIZE = Snes::TilesetTile16::SNES_TILE_SIZE;

    constexpr static unsigned TILE16_ADDR_SHIFT = 7;
    constexpr static unsigned TILE16_ADDR_MASK = 0xffff << TILE16_ADDR_SHIFT;

    static_assert(SNES_TILE16_SIZE == 1 << TILE16_ADDR_SHIFT);

    struct TileBank {
        const unsigned bankId;
        const unsigned startingAddress;
        const uint16_t startingTile16Addr;
        Snes::TilesetTile16 tiles;

        TileBank(unsigned bId, unsigned addr)
            : bankId(bId)
            , startingAddress(addr)
            , startingTile16Addr(addr >> TILE16_ADDR_SHIFT)
            , tiles()
        {
            assert(addr < 0xffffff);
            assert((addr & TILE16_ADDR_MASK) == (addr & 0x7fffff));
        }
    };

    const Project::MemoryMapSettings _memoryMap;
    const unsigned _tilesPerBlock;
    std::unordered_map<Snes::Tile16px, const uint16_t> _map;
    std::vector<TileBank> _tileBanks;

private:
    void createTileBank()
    {
        if (_tileBanks.size() >= _memoryMap.nBanks) {
            throw std::runtime_error("Cannot create Tile Data Bank: not enough ROM Banks");
        }
        const unsigned bankId = _memoryMap.nBanks - _tileBanks.size() - 1;

        _tileBanks.emplace_back(bankId, _memoryMap.bankAddress(bankId));
    }

    inline uint16_t insertTileData(const Snes::Tile16px& tile)
    {
        if (_tileBanks.back().tiles.size() >= _tilesPerBlock) {
            createTileBank();
        }
        auto& tileBank = _tileBanks.back();

        const uint16_t tile16Addr = tileBank.startingTile16Addr + tileBank.tiles.size();

        tileBank.tiles.addTile(tile);

        return tile16Addr;
    }

public:
    RomTileData(const Project::MemoryMapSettings& memoryMap)
        : _memoryMap(memoryMap)
        , _tilesPerBlock(memoryMap.bankSize() / SNES_TILE16_SIZE)
        , _map()
        , _tileBanks()
    {
        _tileBanks.reserve(memoryMap.nBanks);
        createTileBank();
    }

    RomTileData(const RomTileData&) = delete;

    const std::vector<TileBank>& tileBanks() const { return _tileBanks; }

    uint16_t addLargeTile(const Snes::Tile16px& tile)
    {
        const auto it = _map.find(tile);
        if (it != _map.end()) {
            return it->second;
        }
        else {
            return insertTileData(tile);
        }
    }
};
}
}
}
