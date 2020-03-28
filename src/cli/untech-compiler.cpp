/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "helpers/commandlineparser.h"
#include "models/common/file.h"
#include "models/project/project-compiler.h"
#include "models/project/project.h"
#include <cstdlib>
#include <iostream>

using namespace UnTech;
using namespace UnTech::Project;

typedef CommandLine::OptionType OT;
const CommandLine::Config COMMAND_LINE_CONFIG = {
    "UnTech Compiler",
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
    const std::filesystem::path projectFilePath = std::filesystem::u8path(args.filenames().front());
    const std::filesystem::path incFilePath = std::filesystem::u8path(args.options().at("output-inc").string());
    const std::filesystem::path binaryFilePath = std::filesystem::u8path(args.options().at("output-bin").string());

    const std::filesystem::path relativeBinaryFilePath = binaryFilePath.lexically_relative(incFilePath.parent_path());

    std::unique_ptr<ProjectFile> project = loadProjectFile(projectFilePath);
    project->loadAllFiles();

    std::unique_ptr<ProjectOutput> output = compileProject(*project, relativeBinaryFilePath, std::cerr);
    if (!output) {
        std::cerr << "Unable to compile project.\n";
        return EXIT_FAILURE;
    }

    File::atomicWrite(incFilePath, output->incData.str());
    File::atomicWrite(binaryFilePath, output->binaryData);

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
