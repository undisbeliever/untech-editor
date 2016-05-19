#include "models/metasprite-compiler/compiler.h"
#include "models/metasprite-compiler/msexportorder.h"
#include "models/metasprite.h"
#include <cstdlib>
#include <iostream>

using namespace UnTech;

namespace MS = UnTech::MetaSprite;
namespace MSC = UnTech::MetaSpriteCompiler;

// ::TODO version argument::

int main(int argc, char* argv[])
{
    if (argc != 2) {
        auto s = File::splitFilename(argv[0]);

        std::cerr << "UnTech MetaSprite Compiler\n"
                     "\n"
                  << "usage: " << s.second << " <input file>" << std::endl;

        return EXIT_FAILURE;
    }

    const char* filename = argv[1];

    MSC::MsExportOrderDocument eoDocument(filename);

    MSC::Compiler compiler;

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

    compiler.writeToIncFile(std::cout);

    return EXIT_SUCCESS;
}
