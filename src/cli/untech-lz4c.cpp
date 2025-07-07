/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "argparser.h"
#include "models/common/file.h"
#include "models/lz4/lz4.h"
#include <cstdlib>
#include <iostream>

using namespace UnTech;
using namespace UnTech::ArgParser;

struct Args {
    std::filesystem::path inputFilename;

    std::filesystem::path outputFilename;
    unsigned limit;
    bool verbose;
};

// clang-format off
constexpr static auto ARG_PARSER_CONFIG = argParserConfig(
    "UnTech LZ4 HC block compressor."
    "\nWARNING: This compressor uses a modified block frame and is incompatible with the lz4 standard.",

    "input file",

    RequiredArg< &Args::outputFilename  >{  'o',    "output",   "output file"                               },
    OptionalArg< &Args::limit           >{  'l',    "limit",    "limit output file size in bytes",  0xffffU },
    BooleanArg<  &Args::verbose         >{  'v',    "verbose",  "verbose output"                            }
);
// clang-format on

int process(const Args& args)
{
    const auto input = File::readBinaryFile(args.inputFilename, 4 * 1024 * 1024);
    const auto out = lz4HcCompress(input, args.limit);

    File::writeFile(args.outputFilename, out);

    if (args.verbose) {
        double percent = (double)out.size() / (double)input.size() * 100;

        std::cout << "Compressed " << input.size() << " bytes into " << out.size() << " bytes"
                  << " (" << percent << "%)"
                  << '\n';
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
