/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "helpers/commandlineparser.h"
#include "models/common/atomicofstream.h"
#include "models/metasprite/compiler/compiler.h"
#include "models/metasprite/project.h"
#include <cstdlib>
#include <iostream>

using namespace UnTech;
using namespace UnTech::MetaSprite;

const unsigned TBS = Compiler::TilesetCompiler::DEFAULT_TILE_BLOCK_SIZE;

typedef CommandLine::OptionType OT;
const CommandLine::Config COMMAND_LINE_CONFIG = {
    "UnTech MetaSprite Compiler",
    true,
    true,
    false,
    Project::FILE_EXTENSION + " file",
    {
        { 'o', "output", OT::STRING, true, {}, "output file" },
        { 't', "tileblock", OT::UNSIGNED, false, TBS, "tileset block size" },
        { 0, "version", OT::VERSION, false, {}, "display version information" },
        { 'h', "help", OT::HELP, false, {}, "display this help message" },
    }
};

int compile(const CommandLine::Parser& args)
{
    std::unique_ptr<Project> project = loadProject(args.filenames().front());
    for (auto& fs : project->frameSets) {
        fs.loadFile();
    }

    ErrorList errorList;
    Compiler::Compiler compiler(
        errorList,
        args.options().at("tileblock").uint());

    for (auto& fs : project->frameSets) {
        fs.convertSpriteImporter(errorList);

        if (fs.msFrameSet) {
            compiler.processFrameSet(*fs.msFrameSet);
        }
        else {
            compiler.processNullFrameSet();
        }
    }

    for (const auto& w : errorList.warnings) {
        std::cerr << "WARNING: " << w << '\n';
    }

    for (const auto& e : errorList.errors) {
        std::cerr << "ERROR: " << e << '\n';
    }

    if (!errorList.errors.empty()) {
        return EXIT_FAILURE;
    }

    AtomicOfStream os(args.options().at("output").string());

    compiler.writeToIncFile(os);
    compiler.writeToReferencesFile(os);

    os.commit();

    return EXIT_SUCCESS;
}

int main(int argc, const char* argv[])
{
    try {
        CommandLine::Parser args(COMMAND_LINE_CONFIG);
        args.parse(argc, argv);
        return compile(args);
    }
    catch (const std::exception& ex) {
        std::cerr << "ERROR: "
                  << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
}
