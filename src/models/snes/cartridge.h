/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace UnTech {
namespace Snes {
namespace Cartridge {

enum class MemoryMap {
    LOROM,
    HIROM
};

size_t headerAddress(MemoryMap memoryMap);

// returns true if the ROM header matches that of `snes_header.inc`
bool isHeaderValid(const std::vector<uint8_t>& rom, MemoryMap memoryMap);

// throws std::out_of_range if the rom is an invalid size
uint16_t readChecksum(const std::vector<uint8_t>& rom, MemoryMap memoryMap);

// throws std::runtime_error if the rom is an invalid size
uint16_t calculateChecksum(const std::vector<uint8_t>& rom, MemoryMap memoryMap);

// throws exception is unable to write to file
void writeChecksum(const std::string& filename, uint16_t checksum, MemoryMap memoryMap);
}
}
}
