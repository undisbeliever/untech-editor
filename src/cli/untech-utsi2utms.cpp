#include "../models/sprite-importer.h"
#include "../models/metasprite.h"
#include "../models/utsi2utms/utsi2utms.h"
#include "../models/common/file.h"
#include "../models/common/namedlist.h"
#include <iostream>
#include <cstdlib>

using namespace UnTech;

namespace SI = UnTech::SpriteImporter;
namespace MS = UnTech::MetaSprite;

// ::TODO version argument::

int main(int argc, char* argv[])
{
    if (argc != 2) {
        auto s = File::splitFilename(argv[0]);

        std::cerr << "usage: " << s.second << " <input file>" << std::endl;

        return EXIT_FAILURE;
    }

    const char* filename = argv[1];

    UnTech::Utsi2Utms converter;

    SI::SpriteImporterDocument siDocument(filename);
    std::unique_ptr<MS::MetaSpriteDocument> msDocument = converter.convert(siDocument);

    for (const std::string& w : converter.warnings()) {
        std::cerr << "warning: " << w << '\n';
    }

    for (const std::string& e : converter.errors()) {
        std::cerr << "error: " << e << '\n';
    }

    if (msDocument == nullptr) {
        std::cerr << "Error processing frameset.\n";
        return EXIT_FAILURE;
    }

    if (!converter.errors().empty()) {
        return EXIT_FAILURE;
    }

    // Does not use the document's write functions ATM
    // as this app just combines the documents in one sitting.
    MS::Serializer::writeFile(msDocument->frameSet(), std::cout);

    return EXIT_SUCCESS;
}
