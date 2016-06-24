#pragma once

#include <cstdint>
#include <map>
#include <string>

namespace UnTech {
namespace MetaSpriteCommon {

class TilesetType {
public:
    enum class Enum : uint_fast8_t {
        ONE_TILE = 0x00,
        TWO_TILES = 0x02,
        ONE_ROW = 0x04,
        TWO_ROWS = 0x06,
        ONE_TILE_FIXED = 0x80,
        TWO_TILES_FIXED = 0x82,
        ONE_ROW_FIXED = 0x84,
        TWO_ROWS_FIXED = 0x86
    };

    static const std::map<Enum, std::string> enumMap;
    static const std::map<std::string, Enum> stringMap;

    TilesetType(const Enum v = Enum::ONE_TILE)
        : _value(v)
    {
    }

    TilesetType(const std::string str)
        : _value(stringMap.at(str))
    {
    }

    TilesetType(const TilesetType&) = default;

    static TilesetType smallestFixedTileset(unsigned tilesetSize);

    unsigned nTiles() const;
    unsigned tilesetSplitPoint() const;

    Enum value() const { return _value; };
    uint_fast8_t romValue() const { return (uint_fast8_t)_value; }

    const std::string& string() const { return enumMap.at(_value); }
    bool isFixed() const { return (unsigned)_value & 0x80; }

    uint8_t engineValue() const { return (uint8_t)_value; }

    bool operator==(const TilesetType& o) const
    {
        return _value == o._value;
    }
    bool operator==(Enum e) const
    {
        return _value == e;
    }

    bool operator!=(const TilesetType& o) const
    {
        return _value != o._value;
    }
    bool operator!=(Enum e) const
    {
        return _value != e;
    }

private:
    Enum _value;
};
}
}
