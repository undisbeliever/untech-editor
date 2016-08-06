#include "helpers/commandlineparser.h"
#include "models/common/atomicofstream.h"
#include "models/metasprite-compiler/compiler.h"
#include "models/metasprite-compiler/msexportorder.h"
#include "models/metasprite.h"
#include <cstdlib>
#include <iostream>

using namespace UnTech;

namespace MS = UnTech::MetaSprite;
namespace MSC = UnTech::MetaSpriteCompiler;

const unsigned TBS = MSC::Compiler::DEFAULT_TILE_BLOCK_SIZE;

typedef CommandLine::OptionType OT;
const CommandLine::Config COMMAND_LINE_CONFIG = {
    "UnTech MetaSprite Compiler",
    true,
    true,
    false,
    MSC::MsExportOrderDocument::DOCUMENT_TYPE.extension + " file",
    {
        { 'o', "output", OT::STRING, true, {}, "output file" },
        { 't', "tileblock", OT::UNSIGNED, false, TBS, "tileset block size" },
        { 0, "version", OT::VERSION, false, {}, "display version information" },
        { 'h', "help", OT::HELP, false, {}, "display this help message" },
    }
};

int main(int argc, const char* argv[])
{
    CommandLine::Parser args(COMMAND_LINE_CONFIG);
    args.parse(argc, argv);

    MSC::Compiler compiler(
        args.options().at("tileblock").uint());

    MSC::MsExportOrderDocument eoDocument(
        args.filenames().front());

    for (auto& fsd : eoDocument.exportOrder().frameSets()) {
        if (fsd) {
            compiler.processFrameSet(fsd->frameSet());
        }
        else {
            compiler.processNullFrameSet();
        }
    }

    for (const std::string& w : compiler.warnings()) {
        std::cerr << "warning: " << w << '\n';
    }

    for (const std::string& e : compiler.errors()) {
        std::cerr << "error: " << e << '\n';
    }

    if (!compiler.errors().empty()) {
        return EXIT_FAILURE;
    }

    AtomicOfStream os(args.options().at("output").string());

    compiler.writeToIncFile(os);
    compiler.writeToReferencesFile(os);

    os.commit();

    return EXIT_SUCCESS;
}
