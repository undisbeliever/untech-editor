/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "helpers/commandlineparser.h"
#include "models/common/atomicofstream.h"
#include "models/common/errorlist.h"
#include "models/metasprite/compiler/compiler.h"
#include "models/metasprite/compiler/references.h"
#include "models/metasprite/project.h"
#include <cstdlib>
#include <iostream>

using namespace UnTech;
using namespace UnTech::MetaSprite;

const unsigned TBS = Compiler::RomTileData::DEFAULT_TILE_BLOCK_SIZE;

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

static bool validateNamesUnique(const Project& project)
{
    ErrorList errorList;

    project.validateNamesUnique(errorList);

    if (!errorList.empty()) {
        errorList.printIndented(std::cerr);
    }

    return errorList.hasError();
}

int compile(const CommandLine::Parser& args)
{
    std::unique_ptr<Project> project = loadProject(args.filenames().front());
    for (auto& fs : project->frameSets) {
        fs.loadFile();
    }
    project->exportOrders.loadAllFiles();

    // validation is done here to silence export order errors in GUI
    validateNamesUnique(*project);

    bool valid = true;
    Compiler::CompiledRomData romData(args.options().at("tileblock").uint());

    for (auto& fs : project->frameSets) {
        ErrorList errorList;

        fs.convertSpriteImporter(errorList);

        if (fs.msFrameSet) {
            const auto* exportOrder = project->exportOrders.find(fs.msFrameSet->exportOrder);
            processAndSaveFrameSet(*fs.msFrameSet, exportOrder, errorList, romData);
        }
        else {
            processNullFrameSet(romData);
        }

        if (!errorList.empty()) {
            std::cerr << fs.name() << ":\n";
            errorList.printIndented(std::cerr);
        }

        valid &= errorList.hasError() == false;
    }

    if (!valid) {
        return EXIT_FAILURE;
    }

    AtomicOfStream os(args.options().at("output").string());

    romData.writeToIncFile(os);

    Compiler::writeFrameSetReferences(*project, os);
    Compiler::writeExportOrderReferences(*project, os);

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
