/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "helpers/commandlineparser.h"
#include "models/common/exceptions.h"
#include "models/common/file.h"
#include "models/common/string.h"
#include "models/common/u8strings.h"
#include "models/snes/cartridge.h"
#include <cstdlib>
#include <iomanip>
#include <iostream>

using namespace UnTech;
using namespace UnTech::Snes::Cartridge;

using OT = CommandLine::OptionType;
const CommandLine::Config COMMAND_LINE_CONFIG = {
    "UnTech Write Sfc Header Utility",
    "sfc file",
    {
        { 0, "lorom", OT::BOOLEAN, false, {}, "LoROM mapping" },
        { 0, "hirom", OT::BOOLEAN, false, {}, "HiROM mapping" },
        { 'v', "verbose", OT::BOOLEAN, false, {}, "verbose output" },
    }
};

static MemoryMap getMemoryMap(const CommandLine::Parser& args)
{
    if (args.options().at("lorom").boolean()) {
        return MemoryMap::LOROM;
    }
    else if (args.options().at("hirom").boolean()) {
        return MemoryMap::HIROM;
    }
    else {
        throw runtime_error(u8"expected --lorom or --hirom");
    }
}

int process(const CommandLine::Parser& args)
{
    static const std::filesystem::path sfcExtension(".sfc");

    const std::filesystem::path& filename = args.inputFilename();

    if (filename.extension() != sfcExtension) {
        throw runtime_error(u8"Invalid file extension, expected a .sfc file");
    }

    bool verbose = args.options().at("verbose").boolean();

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
            std::cout << "Wrote to " << filename << std::endl;
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
        CommandLine::Parser args(COMMAND_LINE_CONFIG);
        args.parse(argc, argv);
        return process(args);
    }
    catch (const std::exception& ex) {
        std::cerr << "ERROR: "
                  << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
}
