#include "../models/sprite-importer.h"
#include "../models/common/file.h"
#include "../models/common/namedlist.h"
#include <iostream>
#include <cstdlib>

using namespace UnTech;

namespace SI = UnTech::SpriteImporter;

int main(int argc, char* argv[])
{
    if (argc < 2) {
        auto s = File::splitFilename(argv[0]);

        std::cerr << "usage: " << s.second << " <input file(s)>" << std::endl;

        return EXIT_FAILURE;
    }

    // Does not use the document's write functions
    // as this app just combines the documents in one sitting.

    for (int i = 1; i < argc; i++) {
        SI::SpriteImporterDocument document(argv[i]);

        SI::Serializer::writeFile(*document.frameSet(), std::cout);
    }

    return EXIT_SUCCESS;
}
