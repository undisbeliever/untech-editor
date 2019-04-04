/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "helpers/commandlineparser.h"
#include "models/common/errorlist.h"
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
    ErrorList errorList;

    auto siFrameSet = SI::loadFrameSet(infilename);
    auto msFrameSet = MetaSprite::utsi2utms(*siFrameSet, errorList);

    std::cerr << infilename << ":\n";
    errorList.printIndented(std::cerr);

    if (msFrameSet == nullptr) {
        std::cerr << "Error processing frameset.\n";
        return EXIT_FAILURE;
    }

    if (errorList.hasError()) {
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
        std::cerr << "ERROR: "
                  << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
}
