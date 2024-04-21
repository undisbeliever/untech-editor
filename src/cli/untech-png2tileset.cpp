/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "argparser.h"
#include "models/common/exceptions.h"
#include "models/common/indexedimage.h"
#include "models/snes/image2tileset.h"
#include <cstdlib>
#include <iostream>

using namespace UnTech;
using namespace UnTech::Snes;
using namespace UnTech::ArgParser;

struct Args {
    std::filesystem::path inputFilename;

    unsigned bpp;
    std::filesystem::path tilesetFilename;
    std::filesystem::path paletteFilename; // optional
};

// clang-format off
constexpr static auto ARG_PARSER_CONFIG = argParserConfig(
    "UnTech png2tileset",
    "png file",

    RequiredArg< &Args::bpp             >{  'b',    "bpp",      "bits per pixel"        },
    RequiredArg< &Args::tilesetFilename >{  'o',    "output",   "tileset output file"   },
    OptionalArg< &Args::paletteFilename >{  'p',    "palette",  "palette output file"   }
);
// clang-format on

int process(const Args& args)
{
    const auto image = IndexedImage::loadPngImage_shared(args.inputFilename);
    assert(image);

    if (image->empty()) {
        throw runtime_error(image->errorString());
    }

    const auto bitDepth = toBitDepthSpecial(args.bpp);

    ImageToTileset::convertAndSave(*image, bitDepth, args.tilesetFilename, args.paletteFilename);

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
