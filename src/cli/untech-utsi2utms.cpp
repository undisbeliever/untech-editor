#include "helpers/commandlineparser.h"
#include "models/common/file.h"
#include "models/metasprite/utsi2utms/utsi2utms.h"
#include <cstdlib>
#include <iostream>

using namespace UnTech;
namespace MS = UnTech::MetaSprite::MetaSprite;
namespace SI = UnTech::MetaSprite::SpriteImporter;

typedef CommandLine::OptionType OT;
const CommandLine::Config COMMAND_LINE_CONFIG = {
    "UnTech Sprite Importer",
    true,
    true,
    false,
    SI::FrameSet::FILE_EXTENSION + " file",
    {
        { 'o', "output", OT::STRING, true, {}, "output file" },
        { 0, "version", OT::VERSION, false, {}, "display version information" },
        { 'h', "help", OT::HELP, false, {}, "display this help message" },
    }
};

int convert(const std::string& infilename, const std::string& outfilename)
{
    MetaSprite::Utsi2Utms converter;

    auto siFrameSet = SI::loadFrameSet(infilename);
    auto msFrameSet = converter.convert(*siFrameSet);

    for (const std::string& w : converter.warnings()) {
        std::cerr << "warning: " << w << '\n';
    }

    for (const std::string& e : converter.errors()) {
        std::cerr << "error: " << e << '\n';
    }

    if (msFrameSet == nullptr) {
        std::cerr << "Error processing frameset.\n";
        return EXIT_FAILURE;
    }

    if (!converter.errors().empty()) {
        return EXIT_FAILURE;
    }

    MS::saveFrameSet(*msFrameSet, outfilename);

    return EXIT_SUCCESS;
}

int main(int argc, const char* argv[])
{
    CommandLine::Parser args(COMMAND_LINE_CONFIG);
    args.parse(argc, argv);

    try {
        return convert(
            args.filenames().front(),
            args.options().at("output").string());
    }
    catch (const std::exception& ex) {
        std::cerr << ex.what();
        return EXIT_FAILURE;
    }
}
