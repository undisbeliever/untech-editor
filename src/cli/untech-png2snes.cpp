/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "argparser.h"
#include "models/common/exceptions.h"
#include "models/common/file.h"
#include "models/common/indexedimage.h"
#include "models/snes/bit-depth.h"
#include "models/snes/image2snes.h"
#include "models/snes/tile-data.h"
#include <cstdlib>
#include <iostream>

using namespace UnTech;
using namespace UnTech::Snes;
using namespace UnTech::ArgParser;

struct Args {
    std::filesystem::path inputFilename;

    unsigned bpp;
    std::filesystem::path tilesetFilename;
    std::filesystem::path tilemapFilename;
    std::filesystem::path paletteFilename;

    unsigned tileOffset;
    unsigned maxTiles;
    unsigned paletteOffset;
    unsigned maxPalettes;
    unsigned tilemapOrder;

    bool verbose;
};

// clang-format off
constexpr static auto ARG_PARSER_CONFIG = argParserConfig(
    "UnTech png2snes",
    "png file",

    RequiredArg<    &Args::bpp              >{  'b',    "bpp",              "bits per pixel",                       },
    RequiredArg<    &Args::tilesetFilename  >{  't',    "tileset",          "tileset output file"                   },
    RequiredArg<    &Args::tilemapFilename  >{  'm',    "tilemap",          "tilemap output file"                   },
    RequiredArg<    &Args::paletteFilename  >{  'p',    "palette",          "palette output file"                   },
    OptionalArg<    &Args::tileOffset       >{  '\0',   "tile-offset",      "maximum number of tiles",      0U      },
    OptionalArg<    &Args::maxTiles         >{  '\0',   "max-tiles",        "maximum number of tiles",      1024U   },
    OptionalArg<    &Args::paletteOffset    >{  '\0',   "palette-offset",   "palette offset",               0U      },
    OptionalArg<    &Args::maxPalettes      >{  '\0',   "max-palettes",     "maximum number of palettes",   8U      },
    OptionalArg<    &Args::tilemapOrder     >{  '\0',   "order",            "tilemap order",                0U      },
    BooleanArg<     &Args::verbose          >{  'v',    "verbose",          "verbose output"                        }
);
// clang-format on

static int process(const Args& args)
{
    const auto bitDepth = toBitDepthSpecial(args.bpp);

    Image2Snes image2Snes(bitDepth);
    image2Snes.setTileOffset(args.tileOffset);
    image2Snes.setMaxTiles(args.maxTiles);
    image2Snes.setPaletteOffset(args.paletteOffset);
    image2Snes.setMaxPalettes(args.maxPalettes);
    image2Snes.setOrder(args.tilemapOrder);

    const auto image = IndexedImage::loadPngImage_shared(args.inputFilename);
    assert(image);

    if (image->empty()) {
        throw runtime_error(image->errorString());
    }

    if (args.verbose) {
        std::cout << "SETTINGS:\n"
                  << "   Bit Depth:      " << args.bpp << "bpp\n"
                  << "   Max Tiles:      " << args.maxTiles << '\n'
                  << "   Tile Offset:    " << args.tileOffset << '\n';

        if (bitDepth <= Snes::BitDepthSpecial::BD_4BPP) {
            std::cout
                << "   Max Palettes:   " << args.maxPalettes << '\n'
                << "   Palette Offset: " << args.paletteOffset << '\n';
        }

        std::cout << "\nINPUT:\n"
                  << "   " << args.inputFilename << '\n'
                  << "   " << image->size().width << " x " << image->size().height << " px\n"
                  << "   " << image->palette().size() << " colors\n"
                  << std::endl;
    }

    image2Snes.process(*image);

    if (args.verbose) {
        const auto& palette = image2Snes.palette();
        const auto& tileset = image2Snes.tileset();
        const auto& tilemap = image2Snes.tilemap();

        const unsigned colorsPerPalette = colorsForBitDepth(bitDepth);
        unsigned nPalettes = (palette.size() + colorsPerPalette - 1) / colorsPerPalette;

        const char* paletteString = nPalettes == 1 ? "palette" : "palettes";
        const char* tileString = tileset.size() == 1 ? "tile" : "tiles";
        const char* tilemapString = tilemap.nMaps() == 1 ? "tilemap" : "tilemaps";

        std::cout << "OUTPUT:\n"
                  << "   " << nPalettes << ' ' << paletteString << '\n'
                  << "   " << tileset.size() << ' ' << tileString << '\n'
                  << "   " << tilemap.width() << " x " << tilemap.height() << ' ' << tilemapString
                  << std::endl;
    }

    File::atomicWrite(args.tilesetFilename, image2Snes.tilesetSnesData());
    File::atomicWrite(args.tilemapFilename, image2Snes.tilemap().snesData());
    File::atomicWrite(args.paletteFilename, image2Snes.paletteSnesData());

    return EXIT_SUCCESS;
}

int main(int argc, const char* argv[])
{
    try {
        const Args args = parseProgramArguments(ARG_PARSER_CONFIG, argc, argv);
        return process(args);
    }
    catch (const std::exception& ex) {
        std::cerr << "ERROR: "
                  << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
}
