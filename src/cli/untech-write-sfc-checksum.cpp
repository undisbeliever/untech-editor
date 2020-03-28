/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "helpers/commandlineparser.h"
#include "models/common/file.h"
#include "models/common/string.h"
#include "models/snes/cartridge.h"
#include <cstdlib>
#include <iomanip>
#include <iostream>

using namespace UnTech;
using namespace UnTech::Snes::Cartridge;

typedef CommandLine::OptionType OT;
const CommandLine::Config COMMAND_LINE_CONFIG = {
    "UnTech Write Sfc Header Utility",
    true,
    true,
    false,
    "sfc file",
    {
        { 0, "lorom", OT::BOOLEAN, false, {}, "LoROM mapping" },
        { 0, "hirom", OT::BOOLEAN, false, {}, "HiROM mapping" },
        { 'v', "verbose", OT::BOOLEAN, false, {}, "verbose output" },
        { 'h', "help", OT::HELP, false, {}, "display this help message" },
    }
};

int process(const CommandLine::Parser& args)
{
    const std::filesystem::path& filename = std::filesystem::u8path(args.filenames().front());

    if (String::endsWith(filename, ".sfc") == false) {
        throw std::runtime_error("Filename does not end in .sfc");
    }

    bool verbose = args.options().at("verbose").boolean();

    MemoryMap memoryMap;
    if (args.options().at("lorom").boolean()) {
        memoryMap = MemoryMap::LOROM;
    }
    else if (args.options().at("hirom").boolean()) {
        memoryMap = MemoryMap::HIROM;
    }
    else {
        throw std::runtime_error("expected --lorom or --hirom");
    }

    std::vector<uint8_t> rom = File::readBinaryFile(filename, 16 * 1024 * 1024);

    // prevents user from accidentally corrupting a file that was not made by untech-engine.
    if (isHeaderValid(rom, memoryMap) == false) {
        throw std::runtime_error("Could not find header. Header must match `snes_header.inc`");
    }

    uint16_t oldChecksum = readChecksum(rom, memoryMap);
    uint16_t oldComplement = 0xffff ^ oldChecksum;

    uint16_t newChecksum = calculateChecksum(rom, memoryMap);
    uint16_t newComplement = 0xffff ^ newChecksum;

    if (oldChecksum != newChecksum || oldComplement != newComplement) {
        if (verbose) {
            std::cout << std::hex << std::setfill('0')
                      << "old checksum: 0x" << std::setw(4) << oldChecksum
                      << " (complement 0x" << std::setw(4) << oldComplement << ")\n"
                      << "new checksum: 0x" << std::setw(4) << newChecksum
                      << " (complement 0x" << std::setw(4) << newComplement << ")\n";
        }

        writeChecksum(filename, newChecksum, memoryMap);

        if (verbose) {
            std::cout << "Wrote to " << filename << std::endl;
        }
    }
    else {
        if (verbose) {
            std::cout << std::hex << std::setfill('0')
                      << "checksum ok: 0x" << std::setw(4) << newChecksum
                      << " (complement 0x" << std::setw(4) << newComplement << ")\n";
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
