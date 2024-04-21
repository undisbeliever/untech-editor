/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "argparser.h"
#include "models/common/exceptions.h"
#include "models/common/file.h"
#include "models/common/u8strings.h"
#include "models/snes/cartridge.h"
#include <cstdlib>
#include <iomanip>
#include <iostream>

using namespace UnTech;
using namespace UnTech::Snes::Cartridge;
using namespace UnTech::ArgParser;

struct Args {
    std::filesystem::path inputFilename;

    bool lorom;
    bool hirom;
    bool verbose;
};

// clang-format off
constexpr static auto ARG_PARSER_CONFIG = argParserConfig(
    "UnTech Write Sfc Header Utility",
    "sfc file",

    BooleanArg< &Args::lorom   >{  '\0',   "lorom",    "LoROM mapping"  },
    BooleanArg< &Args::hirom   >{  '\0',   "hirom",    "HiROM mapping"  },
    BooleanArg< &Args::verbose >{  'v',    "verbose",  "verbose output" }
);
// clang-format on

static MemoryMap getMemoryMap(const Args& args)
{
    if (args.lorom && args.hirom == false) {
        return MemoryMap::LOROM;
    }
    else if (args.hirom && args.lorom == false) {
        return MemoryMap::HIROM;
    }
    else {
        throw runtime_error(u8"expected --lorom or --hirom");
    }
}

int process(const Args& args)
{
    static const std::filesystem::path sfcExtension(".sfc");

    const std::filesystem::path& filename = args.inputFilename;
    const bool verbose = args.verbose;

    if (filename.extension() != sfcExtension) {
        throw runtime_error(u8"Invalid file extension, expected a .sfc file");
    }

    const MemoryMap memoryMap = getMemoryMap(args);

    std::vector<uint8_t> rom = File::readBinaryFile(filename, 16 * 1024 * 1024);

    // prevents user from accidentally corrupting a file that was not made by untech-engine.
    if (isHeaderValid(rom, memoryMap) == false) {
        throw runtime_error(u8"Could not find header. Header must match `snes_header.inc`");
    }

    uint16_t oldChecksum = readChecksum(rom, memoryMap);
    uint16_t oldComplement = 0xffff ^ oldChecksum;

    uint16_t newChecksum = calculateChecksum(rom, memoryMap);
    uint16_t newComplement = 0xffff ^ newChecksum;

    if (oldChecksum != newChecksum || oldComplement != newComplement) {
        if (verbose) {
            const auto msg = stringBuilder(u8"old checksum: 0x", hex_4(oldChecksum), u8" (complement 0x", hex_4(oldComplement), u8")\n",
                                           u8"new checksum: 0x", hex_4(newChecksum), u8" (complement 0x", hex_4(newComplement), u8")\n");
            stdout_write(msg);
        }

        writeChecksum(filename, newChecksum, memoryMap);

        if (verbose) {
            std::cout << "Wrote to " << filename << '\n';
        }
    }
    else {
        if (verbose) {
            const auto msg = stringBuilder(u8"checksum ok: 0x", hex_4(newChecksum), u8" (complement 0x", hex_4(newComplement), u8")\n");
            stdout_write(msg);
        }
    }

    return EXIT_SUCCESS;
}

int main(int argc, const char* argv[])
{
    try {
        const Args args = parseProgramArguments(ARG_PARSER_CONFIG, argc, argv);
        return process(args);
    }
    catch (const std::exception& ex) {
        std::cerr << "ERROR: " << ex.what() << '\n';
        return EXIT_FAILURE;
    }
}
