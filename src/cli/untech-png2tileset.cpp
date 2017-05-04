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

    switch (args.options().at("bpp").uint()) {
    case 1:
        ImageToTileset<1>::convertAndSave(image, tilesetFile, paletteFile);
        break;

    case 2:
        ImageToTileset<2>::convertAndSave(image, tilesetFile, paletteFile);
        break;

    case 3:
        ImageToTileset<3>::convertAndSave(image, tilesetFile, paletteFile);
        break;

    case 4:
        ImageToTileset<4>::convertAndSave(image, tilesetFile, paletteFile);
        break;

    case 8:
        ImageToTileset<8>::convertAndSave(image, tilesetFile, paletteFile);
        break;

    default:
        throw std::runtime_error("Bad bpp value, expected 1, 2, 3, 4 or 8");
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
