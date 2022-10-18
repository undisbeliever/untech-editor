/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "helpers/commandlineparser.h"
#include "models/common/exceptions.h"
#include "models/common/indexedimage.h"
#include "models/snes/image2tileset.h"
#include <cstdlib>
#include <iostream>

using namespace UnTech;
using namespace UnTech::Snes;

using OT = CommandLine::OptionType;
const CommandLine::Config COMMAND_LINE_CONFIG = {
    "UnTech png2tileset",
    "png file",
    {
        { 'b', "bpp", OT::UNSIGNED, true, {}, "bits per pixel" },
        { 'o', "output", OT::FILENAME, true, {}, "tileset output file" },
        { 'p', "palette", OT::FILENAME, false, {}, "palette output file" },
    }
};

int process(const CommandLine::Parser& args)
{
    const auto image = IndexedImage::loadPngImage_shared(args.inputFilename());
    assert(image);

    if (image->empty()) {
        throw runtime_error(image->errorString());
    }

    const std::filesystem::path& tilesetFile = args.options().at("output").path();
    const std::filesystem::path& paletteFile = args.options().at("palette").path();

    const auto bitDepth = toBitDepthSpecial(args.options().at("bpp").uint());

    ImageToTileset::convertAndSave(*image, bitDepth, tilesetFile, paletteFile);

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
