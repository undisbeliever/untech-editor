/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <cstdint>
#include <vector>

namespace UnTech::Snes {

template <size_t TS>
class Tile;

using Tile8px = Tile<8>;
using Tile16px = Tile<16>;

std::vector<uint8_t> snesTileData1bpp(const std::vector<Tile8px>& tiles);
std::vector<uint8_t> snesTileData2bpp(const std::vector<Tile8px>& tiles);
std::vector<uint8_t> snesTileData3bpp(const std::vector<Tile8px>& tiles);
std::vector<uint8_t> snesTileData4bpp(const std::vector<Tile8px>& tiles);
std::vector<uint8_t> snesTileData8bpp(const std::vector<Tile8px>& tiles);
std::vector<uint8_t> snesTileData(const std::vector<Tile8px>& tiles, const unsigned bitDepth);

std::vector<Tile8px> readSnesTileData1bpp(const std::vector<uint8_t>& in);
std::vector<Tile8px> readSnesTileData2bpp(const std::vector<uint8_t>& in);
std::vector<Tile8px> readSnesTileData3bpp(const std::vector<uint8_t>& in);
std::vector<Tile8px> readSnesTileData4bpp(const std::vector<uint8_t>& in);
std::vector<Tile8px> readSnesTileData8bpp(const std::vector<uint8_t>& in);
std::vector<Tile8px> readSnesTileData(const std::vector<uint8_t>& in, const unsigned bitDepth);

std::vector<uint8_t> snesTileData4bppTile16(const std::vector<Tile16px>& tileset);
std::vector<Tile16px> readSnesTileData4bppTile16(const std::vector<uint8_t>& in);

}
