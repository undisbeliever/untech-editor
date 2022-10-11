/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace UnTech::Snes {

struct TilemapEntry {
    const static unsigned CHAR_MASK = 0x03ff;
    const static unsigned PALETTE_MASK = 0x1c00;

    const static unsigned ORDER_BIT = 0x2000;
    const static unsigned HFLIP_BIT = 0x4000;
    const static unsigned VFLIP_BIT = 0x8000;

    const static unsigned PALETTE_SHIFT = 10;
    const static unsigned ORDER_SHIFT = 13;
    const static unsigned HFLIP_SHIFT = 14;
    const static unsigned VFLIP_SHIFT = 15;

    uint16_t data = 0;

    TilemapEntry() = default;
    TilemapEntry(const TilemapEntry&) = default;
    TilemapEntry(TilemapEntry&&) = default;
    TilemapEntry& operator=(const TilemapEntry&) = default;
    TilemapEntry& operator=(TilemapEntry&&) = default;

    TilemapEntry(unsigned character, unsigned palette, bool order, bool hFlip, bool vFlip)
    {
        data = (character & CHAR_MASK)
               | ((palette << PALETTE_SHIFT) & PALETTE_MASK)
               | ((order << ORDER_SHIFT))
               | ((hFlip << HFLIP_SHIFT))
               | ((vFlip << VFLIP_SHIFT));
    }

    [[nodiscard]] unsigned character() const { return data & CHAR_MASK; }
    void setCharacter(unsigned c) { data = (data & ~CHAR_MASK) | (c & CHAR_MASK); }

    [[nodiscard]] unsigned palette() const { return (data & PALETTE_MASK) >> PALETTE_SHIFT; }
    void setPalette(unsigned c) { data = (data & ~PALETTE_MASK) | ((c << PALETTE_SHIFT) & PALETTE_MASK); }

    [[nodiscard]] bool order() const { return data & ORDER_BIT; }
    void setOrder(bool o) { data = (data & ~ORDER_BIT) | (o << ORDER_SHIFT); }

    [[nodiscard]] bool hFlip() const { return data & HFLIP_BIT; }
    void setHFlip(bool o) { data = (data & ~HFLIP_BIT) | (o << HFLIP_SHIFT); }

    [[nodiscard]] bool vFlip() const { return data & VFLIP_BIT; }
    void setVFlip(bool o) { data = (data & ~VFLIP_BIT) | (o << VFLIP_SHIFT); }
};

class Tilemap {

public:
    const static unsigned MAP_SIZE = 32;
    using map_t = std::array<TilemapEntry, MAP_SIZE * MAP_SIZE>;

private:
    unsigned _width;
    unsigned _height;
    std::vector<map_t> _maps;

public:
    explicit Tilemap(unsigned width = 1, unsigned height = 1);
    Tilemap(const Tilemap&) = default;
    Tilemap(Tilemap&&) = default;
    Tilemap& operator=(const Tilemap&) = default;
    Tilemap& operator=(Tilemap&&) = default;

    map_t& map(unsigned i = 0) { return _maps.at(i); }
    [[nodiscard]] const map_t& map(unsigned i = 0) const { return _maps.at(i); }

    map_t& map(unsigned x, unsigned y) { return _maps.at(y * _width + x); }
    [[nodiscard]] const map_t& map(unsigned x, unsigned y) const { return _maps.at(y * _width + x); }

    [[nodiscard]] unsigned nMaps() const { return _width * _height; }
    [[nodiscard]] unsigned width() const { return _width; }
    [[nodiscard]] unsigned height() const { return _height; }

    [[nodiscard]] std::vector<uint8_t> snesData() const;
};

}
