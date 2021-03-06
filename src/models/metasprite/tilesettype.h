/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/enummap.h"
#include <cstdint>

namespace UnTech::MetaSprite {

class TilesetType {
public:
    enum class Enum : uint_fast8_t {
        ONE_TILE_FIXED = 0x00,
        TWO_TILES_FIXED = 0x02,
        ONE_ROW_FIXED = 0x04,
        TWO_ROWS_FIXED = 0x06,
        ONE_TILE = 0x08,
        TWO_TILES = 0x0A,
        ONE_ROW = 0x0C,
        TWO_ROWS = 0x0E
    };

    static const EnumMap<Enum> enumMap;

    TilesetType(const Enum v = Enum::ONE_TILE)
        : _value(v)
    {
    }

    TilesetType(const std::string str)
        : _value(enumMap.valueOf(str))
    {
    }

    static TilesetType smallestFixedTileset(unsigned tilesetSize);

    unsigned nTiles() const;
    unsigned tilesetSplitPoint() const;

    Enum value() const { return _value; }
    uint_fast8_t romValue() const { return (uint_fast8_t)_value; }

    const std::string& string() const { return enumMap.nameOf(_value); }
    bool isFixed() const { return (unsigned)_value < 0x08; }

    uint8_t engineValue() const { return (uint8_t)_value; }

    inline operator Enum() const { return _value; }

    inline bool operator==(const TilesetType& o) const { return _value == o._value; }
    inline bool operator==(Enum e) const { return _value == e; }

    inline bool operator!=(const TilesetType& o) const { return _value != o._value; }
    inline bool operator!=(Enum e) const { return _value != e; }

private:
    Enum _value;
};

}
