/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace UnTech::Snes::Cartridge {

enum class MemoryMap {
    LOROM,
    HIROM
};

size_t headerAddress(MemoryMap memoryMap);

// returns true if the ROM header matches that of `snes_header.inc`
bool isHeaderValid(const std::vector<uint8_t>& rom, MemoryMap memoryMap);

// throws `out_of_range` if the rom is an invalid size
uint16_t readChecksum(const std::vector<uint8_t>& rom, MemoryMap memoryMap);

// throws `runtime_error` if the rom is an invalid size
uint16_t calculateChecksum(const std::vector<uint8_t>& rom, MemoryMap memoryMap);

// throws an exception if writeChecksum is unable to write to file
void writeChecksum(const std::filesystem::path& filename, uint16_t checksum, MemoryMap memoryMap);

}
