/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
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
                     std::string tilePrefix)
        : _entries()
        , _label(std::move(label))
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
    RomTileData(const std::string& blockPrefix, unsigned blockSize)
        : _blockPrefix(blockPrefix)
        , _tilesPerBlock(blockSize / SNES_TILE16_SIZE)
        , _map()
        , _tileBlocks()
    {
        if (_tilesPerBlock < 16) {
            throw std::invalid_argument("block size is too small");
        }
        _tileBlocks.emplace_back();
    }

    RomTileData(const RomTileData&) = delete;

    const std::string& blockPrefix() const { return _blockPrefix; }
    unsigned nBlocks() const { return _tileBlocks.size(); }

    void writeAssertsToIncFile(std::ostream& out) const;
    std::vector<std::vector<uint8_t>> data() const;

    Accessor addLargeTile(const Snes::Tile16px& tile)
    {
        const auto it = _map.find(tile);
        if (it != _map.end()) {
            return it->second;
        }
        else {
            TileAddress addr = insertTileData(tile);

            // unordered_map will ignore insert if tile pattern already exists
            // Thus symmetrical tiles will prefer the unflipped tile.

            _map.emplace(tile, Accessor(addr, false, false));
            _map.emplace(tile.hFlip(), Accessor(addr, true, false));
            _map.emplace(tile.vFlip(), Accessor(addr, false, true));
            _map.emplace(tile.hvFlip(), Accessor(addr, true, true));

            return { addr, false, false };
        }
    }

private:
    inline TileAddress insertTileData(const Snes::Tile16px& tile)
    {
        if (_tileBlocks.back().size() >= _tilesPerBlock) {
            _tileBlocks.emplace_back();
        }

        auto& tileset = _tileBlocks.back();

        unsigned block = _tileBlocks.size() - 1;
        unsigned offset = tileset.size() * SNES_TILE16_SIZE;

        _tileBlocks.back().addTile(tile);

        return TileAddress{ block, offset };
    }

private:
    const std::string _blockPrefix;
    const unsigned _tilesPerBlock;
    std::unordered_map<Snes::Tile16px, const Accessor> _map;
    std::vector<Snes::TilesetTile16> _tileBlocks;
};
}
}
}
