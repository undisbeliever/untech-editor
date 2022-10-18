/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "helpers/commandlineparser.h"
#include "models/common/file.h"
#include "models/common/stringstream.h"
#include "models/common/u8strings.h"
#include "models/project/project-compiler.h"
#include "models/project/project.h"
#include <cstdlib>
#include <iostream>

using namespace UnTech;
using namespace UnTech::Project;

using OT = CommandLine::OptionType;
const CommandLine::Config COMMAND_LINE_CONFIG = {
    "UnTech Compiler",
    "utproject file",
    {
        { 0, "output-inc", OT::FILENAME, true, {}, "output inc file" },
        { 0, "output-bin", OT::FILENAME, true, {}, "output bin file" },
    }
};

int compile(const CommandLine::Parser& args)
{
    const std::filesystem::path& projectFilePath = args.inputFilename();
    const std::filesystem::path& incFilePath = args.options().at("output-inc").path();
    const std::filesystem::path& binaryFilePath = args.options().at("output-bin").path();

    const std::filesystem::path& relativeBinaryFilePath = binaryFilePath.lexically_relative(incFilePath.parent_path());

    std::unique_ptr<ProjectFile> project = loadProjectFile(projectFilePath);
    project->loadAllFiles();

    StringStream errorStream;

    std::unique_ptr<ProjectOutput> output = compileProject(*project, relativeBinaryFilePath, errorStream);

    // Print errors
    if (errorStream.size() != 0) {
        stderr_write(errorStream.string_view());
    }

    if (!output) {
        std::cerr << "Unable to compile project.\n";
        return EXIT_FAILURE;
    }

    File::atomicWrite(incFilePath, output->incData);
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
