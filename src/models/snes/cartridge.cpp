/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "cartridge.h"
#include "models/common/exceptions.h"
#include "models/common/iterators.h"
#include "models/common/stringbuilder.h"
#include <array>
#include <cassert>
#include <climits>
#include <fstream>

namespace UnTech::Snes::Cartridge {

constexpr size_t HEADER_ADDR = 0xffb0;
constexpr size_t CHECKSUM_COMPLEMENT_ADDR = 0xffdc;
constexpr size_t CHECKSUM_ADDR = 0xffde;

constexpr unsigned MIN_ROM_SIZE = 64 * 1024;
constexpr unsigned MAX_ROM_SIZE = 4 * 1024 * 1024;

#define MIN_ROM_STRING "64 KiB"
#define MAX_ROM_STRING "4 MiB"

size_t headerAddress(MemoryMap memoryMap)
{
    switch (memoryMap) {
    case MemoryMap::LOROM:
        return HEADER_ADDR - 0x8000;

    case MemoryMap::HIROM:
        return HEADER_ADDR;
    }

    throw invalid_argument(u8"invalid MemoryMap");
}

static inline size_t checksumCompelementAddress(MemoryMap memoryMap)
{
    constexpr size_t offset = CHECKSUM_COMPLEMENT_ADDR - HEADER_ADDR;
    return headerAddress(memoryMap) + offset;
}

static inline size_t checksumAddress(MemoryMap memoryMap)
{
    constexpr size_t offset = CHECKSUM_ADDR - HEADER_ADDR;
    return headerAddress(memoryMap) + offset;
}

bool isHeaderValid(const std::vector<uint8_t>& rom, MemoryMap memoryMap)
{
    // Blank Maker Code and Game Code as my homebrew stuff is unlicensed.
    static const std::array<uint8_t, 2 + 4 + 7> EXPECTED(
        { { ' ', ' ', ' ', ' ', ' ', ' ', 0, 0, 0, 0, 0, 0, 0 } });

    // addresses of blank words in the interrupt vector
    static const std::array<size_t, 6> BLANK_INTERRUPT_VECTORS(
        { { 0xffe0, 0xffe2, 0xffec,
            0xfff0, 0xfff2, 0xfff6 } });

    if (rom.size() < MIN_ROM_SIZE) {
        return false;
    }

    size_t addr = headerAddress(memoryMap);

    if (!std::equal(EXPECTED.begin(), EXPECTED.end(), rom.begin() + addr)) {
        return false;
    }

    for (size_t v : BLANK_INTERRUPT_VECTORS) {
        size_t a = addr + v - HEADER_ADDR;

        if (rom[a] != 0 || rom[a + 1] != 0) {
            return false;
        }
    }

    return true;
}

uint16_t readChecksum(const std::vector<uint8_t>& rom, MemoryMap memoryMap)
{
    unsigned addr = checksumAddress(memoryMap);

    return rom.at(addr) | rom.at(addr + 1) << 8;
}

uint16_t calculateChecksum(const std::vector<uint8_t>& rom, MemoryMap memoryMap)
{
    static_assert(sizeof(int) > sizeof(uint16_t) + 1, u8"int too small");
    static_assert(INT_MAX > MAX_ROM_SIZE * 256, u8"int too small");

    if (rom.size() < MIN_ROM_SIZE) {
        throw runtime_error(u8"ROM is to small (minimum " MIN_ROM_STRING u8").");
    }
    if (rom.size() > MAX_ROM_SIZE) {
        throw runtime_error(u8"ROM is to large (maximum " MAX_ROM_STRING u8").");
    }

    unsigned part1Size = 1;
    while (part1Size <= rom.size()) {
        part1Size <<= 1;
    }
    part1Size >>= 1;

    int part1 = 0;
    assert(part1Size <= rom.size());
    for (const auto i : range(part1Size)) {
        part1 += rom[i];
    }

    int part2 = 0;
    unsigned part2Count = 0;
    if (part1Size != rom.size()) {
        unsigned part2Size = rom.size() - part1Size;

        part2Count = part1Size / part2Size;
        if (part1Size % part2Size != 0) {
            throw runtime_error(u8"Invalid ROM size.");
        }

        for (const auto i : range(part1Size, rom.size())) {
            part2 += rom[i];
        }
    }

    int oldSum = 0;
    static_assert(CHECKSUM_ADDR == CHECKSUM_COMPLEMENT_ADDR + 2, u8"assumption failed");
    unsigned ccAddr = checksumCompelementAddress(memoryMap);
    for (const auto i : range(4)) {
        oldSum += rom[i + ccAddr];
    }

    int checkSum = part1 + part2 * part2Count - oldSum + 0xff * 2;
    while (checkSum < 0) {
        checkSum += 0x10000;
    }

    return checkSum & 0xffff;
}

void writeChecksum(const std::filesystem::path& filename, uint16_t checksum, MemoryMap memoryMap)
{
    unsigned checksumAddr = checksumAddress(memoryMap);
    unsigned complementAddr = checksumCompelementAddress(memoryMap);

    uint16_t complement = 0xffff ^ checksum;

    std::ofstream out(filename, std::ios::out | std::ios::in | std::ios::binary);
    if (!out) {
        throw runtime_error(u8"Error opening file: ", filename.u8string());
    }

    out.exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);

    out.seekp(complementAddr);
    out.put(complement & 0xff);
    out.put(complement >> 8 & 0xff);

    assert(checksumAddr == complementAddr + 2);
    out.put(checksum & 0xff);
    out.put(checksum >> 8 & 0xff);

    out.close();
}

}
