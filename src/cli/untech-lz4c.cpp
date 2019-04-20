/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "helpers/commandlineparser.h"
#include "models/common/file.h"
#include "models/lz4/lz4.h"
#include <cstdlib>
#include <iostream>

using namespace UnTech;

typedef CommandLine::OptionType OT;
const CommandLine::Config COMMAND_LINE_CONFIG = {
    "UnTech LZ4 HC block compressor."
    "\nWARNING: This compressor uses a modified block frame and is incompatible with the lz4 standard.",
    true,
    true,
    false,
    "input file",
    {
        { 'o', "output", OT::STRING, true, {}, "output file" },
        { 'l', "limit", OT::UNSIGNED, false, 0xffffU, "limit output file size in bytes" },
        { 'v', "verbose", OT::BOOLEAN, false, {}, "verbose output" },
        { '\0', "version", OT::VERSION, false, {}, "display version information" },
        { 'h', "help", OT::HELP, false, {}, "display this help message" },
    }
};

int process(const CommandLine::Parser& args)
{
    const std::string& inputFilename = args.filenames().front();
    const std::string& outputFilename = args.options().at("output").string();
    const unsigned limit = args.options().at("limit").uint();
    const bool verbose = args.options().at("verbose").boolean();

    const auto input = File::readBinaryFile(inputFilename, 4 * 1024 * 1024);
    const auto out = lz4HcCompress(input, limit);

    File::atomicWrite(outputFilename, out);

    if (verbose) {
        double percent = (double)out.size() / (double)input.size() * 100;

        std::cout << "Compressed " << input.size() << " bytes into " << out.size() << " bytes"
                  << " (" << percent << "%)"
                  << std::endl;
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
