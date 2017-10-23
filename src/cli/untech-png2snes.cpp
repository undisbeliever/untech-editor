/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "helpers/commandlineparser.h"
#include "models/common/file.h"
#include "models/common/indexedimage.h"
#include "models/snes/image2snes.h"
#include <cstdlib>
#include <iostream>

using namespace UnTech;
using namespace UnTech::Snes;

typedef CommandLine::OptionType OT;
const CommandLine::Config COMMAND_LINE_CONFIG = {
    "UnTech png2snes",
    true,
    true,
    false,
    "png file",
    {
        { 'b', "bpp", OT::UNSIGNED, true, {}, "bits per pixel" },
        { 't', "tileset", OT::STRING, true, {}, "tileset output file" },
        { 'm', "tilemap", OT::STRING, true, {}, "tilemap output file" },
        { 'p', "palette", OT::STRING, true, {}, "palette output file" },
        { '\0', "tile-offset", OT::UNSIGNED, false, 0U, "tilemap char offset" },
        { '\0', "max-tiles", OT::UNSIGNED, false, 1024U, "maximum number of tiles" },
        { '\0', "palette-offset", OT::UNSIGNED, false, 0U, "palette offset" },
        { '\0', "max-palettes", OT::UNSIGNED, false, 8U, "maximum number of palettes" },
        { '\0', "order", OT::UNSIGNED, false, 0U, "tilemap order" },
        { 'v', "verbose", OT::BOOLEAN, false, {}, "verbose output" },
        { '\0', "version", OT::VERSION, false, {}, "display version information" },
        { 'h', "help", OT::HELP, false, {}, "display this help message" },
    }
};

int process(const CommandLine::Parser& args)
{
    const std::string& inputFile = args.filenames().front();

    const unsigned bitDepth = args.options().at("bpp").uint();
    const unsigned tileOffset = args.options().at("tile-offset").uint();
    const unsigned maxTiles = args.options().at("max-tiles").uint();
    const unsigned paletteOffset = args.options().at("palette-offset").uint();
    const unsigned maxPalettes = args.options().at("max-palettes").uint();
    const bool order = args.options().at("order").uint();

    const bool verbose = args.options().at("verbose").boolean();

    Image2Snes image2Snes(bitDepth);
    image2Snes.setTileOffset(tileOffset);
    image2Snes.setMaxTiles(maxTiles);
    image2Snes.setPaletteOffset(paletteOffset);
    image2Snes.setMaxPalettes(maxPalettes);
    image2Snes.setOrder(order);

    IndexedImage image;
    image.loadPngImage(inputFile);

    if (image.empty()) {
        throw std::runtime_error(image.errorString());
    }

    if (verbose) {
        std::cout << "SETTINGS:\n"
                  << "   Bit Depth:      " << bitDepth << "bpp\n"
                  << "   Max Tiles:      " << maxTiles << '\n'
                  << "   Tile Offset:    " << tileOffset << '\n';

        if (bitDepth <= 4) {
            std::cout
                << "   Max Palettes:   " << maxPalettes << '\n'
                << "   Palette Offset: " << paletteOffset << '\n';
        }

        std::cout << "\nINPUT:\n"
                  << "   " << inputFile << '\n'
                  << "   " << image.size().width << " x " << image.size().height << " px\n"
                  << "   " << image.palette().size() << " colors\n"
                  << std::endl;
    }

    image2Snes.process(image);

    if (verbose) {
        const auto& palette = image2Snes.palette();
        const auto& tileset = image2Snes.tileset();
        const auto& tilemap = image2Snes.tilemap();

        const unsigned colorsPerPalette = 1 << bitDepth;
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

    const std::string& tilesetFile = args.options().at("tileset").string();
    const std::string& tilemapFile = args.options().at("tilemap").string();
    const std::string& paletteFile = args.options().at("palette").string();

    File::atomicWrite(tilesetFile, image2Snes.tileset().snesData());
    File::atomicWrite(tilemapFile, image2Snes.tilemap().snesData());
    File::atomicWrite(paletteFile, image2Snes.paletteSnesData());

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
