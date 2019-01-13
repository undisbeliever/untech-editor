/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "helpers/commandlineparser.h"
#include "models/common/file.h"
#include "models/project/project.h"
#include "models/resources/resources.h"
#include <cstdlib>
#include <iostream>

using namespace UnTech;
using namespace UnTech::Project;
using namespace UnTech::Resources;

typedef CommandLine::OptionType OT;
const CommandLine::Config COMMAND_LINE_CONFIG = {
    "UnTech Resource Compiler",
    true,
    true,
    false,
    ProjectFile::FILE_EXTENSION + " file",
    {
        { 0, "output-inc", OT::STRING, true, {}, "output inc file" },
        { 0, "output-bin", OT::STRING, true, {}, "output bin file" },
        { 0, "version", OT::VERSION, false, {}, "display version information" },
        { 'h', "help", OT::HELP, false, {}, "display this help message" },
    }
};

int compile(const CommandLine::Parser& args)
{
    const std::string& projectFilename = args.filenames().front();
    const std::string& incFilename = args.options().at("output-inc").string();
    const std::string& binaryFilename = args.options().at("output-bin").string();

    const std::string relativeBinaryFilename = File::relativePath(incFilename, binaryFilename);

    std::unique_ptr<ProjectFile> project = loadProjectFile(projectFilename);
    project->loadAllFiles();

    std::unique_ptr<ResourcesOutput> output = compileResources(*project, relativeBinaryFilename, std::cerr);
    if (!output) {
        std::cerr << "Unable to compile resources.\n";
        return EXIT_FAILURE;
    }

    File::atomicWrite(incFilename, output->incData);
    File::atomicWrite(binaryFilename, output->binaryData);

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
