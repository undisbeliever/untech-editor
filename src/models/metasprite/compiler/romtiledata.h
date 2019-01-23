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

struct TileAddress {
    unsigned block = 0;
    unsigned offset = 0;

    bool operator==(const TileAddress& o) const
    {
        return block == o.block && offset == o.offset;
    }
};

class RomDmaTile16Entry {
    unsigned _tileCount = 0;
    std::array<TileAddress, 16> _tiles;

public:
    unsigned dataSize() const { return _tileCount * 2 + 1; }

    void addTile(const TileAddress& a)
    {
        _tiles.at(_tileCount) = a;
        _tileCount++;
    }

    void writeToIncFile(std::ostream& out, const std::string& tilePrefix) const;

    bool operator==(const RomDmaTile16Entry& o) const
    {
        return _tileCount == o._tileCount && _tiles == o._tiles;
    }
};

class RomDmaTile16Data {
    std::vector<RomDmaTile16Entry> _entries;
    const std::string _label;
    const std::string _segmentName;
    const std::string _tilePrefix;

public:
    RomDmaTile16Data(std::string label,
                     std::string segmentName,
                     std::string tilePrefix)
        : _entries()
        , _label(std::move(label))
        , _segmentName(std::move(segmentName))
        , _tilePrefix(std::move(tilePrefix))
    {
    }

    IndexPlusOne addEntry(const RomDmaTile16Entry& entry)
    {
        unsigned size = 0;

        for (const RomDmaTile16Entry& e : _entries) {
            if (e == entry) {
                return IndexPlusOne{ size + 1 };
            }
            size += e.dataSize();
        }

        _entries.emplace_back(entry);
        return IndexPlusOne{ size + 1 };
    }

    void writeToIncFile(std::ostream& out) const;
};

class RomTileData {
public:
    const static unsigned BYTES_PER_LINE = 16;
    constexpr static unsigned SNES_TILE16_SIZE = Snes::TilesetTile16::SNES_TILE_SIZE;

    constexpr static unsigned DEFAULT_TILE_BLOCK_SIZE = 8 * 1024;

    struct Accessor {
        Accessor(const TileAddress& addr, bool hFlip, bool vFlip)
            : addr(addr)
            , hFlip(hFlip)
            , vFlip(vFlip)
        {
        }

        TileAddress addr;
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
    {
        if (blockSize < SNES_TILE16_SIZE) {
            throw std::invalid_argument("block size is too small");
        }
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
            TileAddress addr = insertTileData(tile);

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
    inline TileAddress insertTileData(const Snes::Tile16px& tile)
    {
        if (_currentOffset >= _blockSize) {
            _currentOffset = 0;
            _currentBlock++;
        }
        unsigned offset = _currentOffset;
        _currentOffset += SNES_TILE16_SIZE;

        _tiles.addTile(tile);

        return TileAddress{ _currentBlock, offset };
    }

private:
    const std::string _blockPrefix;
    const std::string _segmentPrefix;
    unsigned _blockSize;
    unsigned _currentBlock;
    unsigned _currentOffset;
    std::unordered_map<Snes::Tile16px, const Accessor> _map;
    Snes::TilesetTile16 _tiles;
};
}
}
}
