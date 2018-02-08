/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "romdata.h"
#include "models/snes/tileset.h"
#include <cassert>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

// uses std::string instead of idstring as
// the assembly data labels can contain dots (.)

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

class RomTileData {
public:
    const static unsigned BYTES_PER_LINE = 16;
    constexpr static unsigned SNES_TILE16_SIZE = Snes::TilesetTile16::SNES_TILE_SIZE;

    struct Accessor {
        Accessor(const RomOffsetPtr& addr, bool hFlip, bool vFlip)
            : addr(addr)
            , hFlip(hFlip)
            , vFlip(vFlip)
        {
        }

        RomOffsetPtr addr;
        const bool hFlip;
        const bool vFlip;
    };

public:
    RomTileData(const std::string& blockPrefix, const std::string& segmentPrefix, unsigned blockSize)
        : _blockPrefix(blockPrefix)
        , _segmentPrefix(segmentPrefix)
        , _blockSize(blockSize)
        , _currentBlock(0)
        , _currentOffset(0)
        , _map()
        , _tiles()
        , _blockNames()
    {
        if (blockSize < SNES_TILE16_SIZE) {
            throw std::invalid_argument("block size is too small");
        }

        _blockNames.emplace_back(std::make_unique<std::string>(blockPrefix + "_0"));
    }

    RomTileData(const RomTileData&) = delete;

    void writeToIncFile(std::ostream& out) const;

    const Accessor& addLargeTile(const Snes::Tile16px& tile)
    {
        const auto it = _map.find(tile);
        if (it != _map.end()) {
            return it->second;
        }
        else {
            RomOffsetPtr addr = insertTileData(tile);

            // unordered_map will ignore insert if tile pattern already exists
            // Thus symmetrical tiles will prefer the unflipped tile.

            auto ret = _map.emplace(tile, Accessor(addr, false, false));

            _map.emplace(tile.hFlip(), Accessor(addr, true, false));
            _map.emplace(tile.vFlip(), Accessor(addr, false, true));
            _map.emplace(tile.hvFlip(), Accessor(addr, true, true));

            return ret.first->second;
        }
    }

private:
    inline RomOffsetPtr insertTileData(const Snes::Tile16px& tile)
    {
        RomOffsetPtr ret(_blockNames[_currentBlock].get(), _currentOffset);

        _tiles.addTile(tile);

        _currentOffset += SNES_TILE16_SIZE;
        if (_currentOffset >= _blockSize) {
            _currentOffset = 0;

            _currentBlock++;
            _blockNames.emplace_back(std::make_unique<std::string>(
                _blockPrefix + '_' + std::to_string(_currentBlock)));
        }

        return ret;
    }

private:
    const std::string _blockPrefix;
    const std::string _segmentPrefix;
    uint32_t _blockSize;
    uint32_t _currentBlock;
    uint32_t _currentOffset;
    std::unordered_map<Snes::Tile16px, const Accessor> _map;
    Snes::TilesetTile16 _tiles;

    // RomOffsetPtr uses string pointers
    std::vector<std::unique_ptr<std::string>> _blockNames;
};
}
}
}
