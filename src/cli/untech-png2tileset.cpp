/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "helpers/commandlineparser.h"
#include "models/common/indexedimage.h"
#include "models/snes/image2tileset.h"
#include <cstdlib>
#include <iostream>

using namespace UnTech;
using namespace UnTech::Snes;

typedef CommandLine::OptionType OT;
const CommandLine::Config COMMAND_LINE_CONFIG = {
    "UnTech png2tileset",
    true,
    true,
    false,
    "png file",
    {
        { 'b', "bpp", OT::UNSIGNED, true, {}, "bits per pixel" },
        { 'o', "output", OT::STRING, true, {}, "tileset output file" },
        { 'p', "palette", OT::STRING, false, {}, "palette output file" },
        { '\0', "version", OT::VERSION, false, {}, "display version information" },
        { 'h', "help", OT::HELP, false, {}, "display this help message" },
    }
};

int process(const CommandLine::Parser& args)
{
    IndexedImage image;
    image.loadPngImage(args.filenames().front());

    if (image.empty()) {
        throw std::runtime_error(image.errorString());
    }

    const std::string& tilesetFile = args.options().at("output").string();
    const std::string& paletteFile = args.options().at("palette").string();

    const unsigned bitDepth = args.options().at("bpp").uint();

    ImageToTileset::convertAndSave(image, bitDepth, tilesetFile, paletteFile);

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
