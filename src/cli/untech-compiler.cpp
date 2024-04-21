/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "argparser.h"
#include "models/common/file.h"
#include "models/common/stringstream.h"
#include "models/common/u8strings.h"
#include "models/project/project-compiler.h"
#include "models/project/project.h"
#include <cstdlib>
#include <iostream>

using namespace UnTech;
using namespace UnTech::Project;
using namespace UnTech::ArgParser;

struct Args {
    std::filesystem::path inputFilename;

    std::filesystem::path outputIncFilename;
    std::filesystem::path outputBinFilename;
};

// clang-format off
constexpr static auto ARG_PARSER_CONFIG = argParserConfig(
    "UnTech Compiler",
    "utproject file",

    RequiredArg< &Args::outputIncFilename   >{  '\0',   "output-inc",  "output inc file"   },
    RequiredArg< &Args::outputBinFilename   >{  '\0',   "output-bin",  "output bin file"   }
);
// clang-format on

int compile(const Args& args)
{
    const std::filesystem::path& relativeBinaryFilePath = args.outputBinFilename.lexically_relative(args.outputIncFilename.parent_path());

    std::unique_ptr<ProjectFile> project = loadProjectFile(args.inputFilename);
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

    File::atomicWrite(args.outputIncFilename, output->incData);
    File::atomicWrite(args.outputBinFilename, output->binaryData);

    return EXIT_SUCCESS;
}

int main(int argc, const char* argv[])
{
    try {
        const Args args = parseProgramArguments(ARG_PARSER_CONFIG, argc, argv);
        return compile(args);
    }
    catch (const std::exception& ex) {
        std::cerr << "ERROR: " << ex.what() << '\n';
        return EXIT_FAILURE;
    }
}
