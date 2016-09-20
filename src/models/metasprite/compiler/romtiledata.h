#pragma once

#include "romdata.h"
#include "models/snes/tile.h"
#include "models/snes/tile.hpp"
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
        if (blockSize < Snes::Tile4bpp16px::SNES_DATA_SIZE) {
            throw std::invalid_argument("block size is too small");
        }

        _blockNames.emplace_back(std::make_unique<std::string>(blockPrefix + "_0"));
    }

    RomTileData(const RomTileData&) = delete;

    void writeToIncFile(std::ostream& out) const;

    const Accessor& addLargeTile(const Snes::Tile4bpp16px& tile)
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

            _map.emplace(tile.hFlip(), Accessor(addr, false, true));
            _map.emplace(tile.vFlip(), Accessor(addr, true, false));
            _map.emplace(tile.hvFlip(), Accessor(addr, true, true));

            return ret.first->second;
        }
    }

private:
    inline RomOffsetPtr insertTileData(const Snes::Tile4bpp16px& tile)
    {
        RomOffsetPtr ret(_blockNames[_currentBlock].get(), _currentOffset);

        _tiles.push_back(tile);

        _currentOffset += Snes::Tile4bpp16px::SNES_DATA_SIZE;
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
    std::unordered_map<Snes::Tile4bpp16px, const Accessor> _map;
    std::vector<Snes::Tile4bpp16px> _tiles;

    // RomOffsetPtr uses string pointers
    std::vector<std::unique_ptr<std::string>> _blockNames;
};
}
}
}
