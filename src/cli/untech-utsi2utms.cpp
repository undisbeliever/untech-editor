#include "helpers/commandlineparser.h"
#include "models/common/file.h"
#include "models/common/namedlist.h"
#include "models/metasprite.h"
#include "models/metasprite/serializer.h"
#include "models/sprite-importer.h"
#include "models/utsi2utms/utsi2utms.h"
#include <cstdlib>
#include <iostream>

using namespace UnTech;

namespace SI = UnTech::SpriteImporter;
namespace MS = UnTech::MetaSprite;

typedef CommandLine::OptionType OT;

typedef CommandLine::OptionType OT;
const CommandLine::Config COMMAND_LINE_CONFIG = {
    "UnTech Sprite Importer",
    true,
    true,
    false,
    SI::SpriteImporterDocument::DOCUMENT_TYPE.extension + " file",
    {
        { 'o', "output", OT::STRING, true, {}, "output file" },
        { 0, "version", OT::VERSION, false, {}, "display version information" },
        { 'h', "help", OT::HELP, false, {}, "display this help message" },
    }
};

int main(int argc, const char* argv[])
{
    CommandLine::Parser args(COMMAND_LINE_CONFIG);
    args.parse(argc, argv);

    UnTech::Utsi2Utms converter;

    SI::SpriteImporterDocument siDocument(
        args.filenames().front());

    auto msDocument = converter.convert(siDocument);

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

    msDocument->saveFile(args.options().at("output").string());

    return EXIT_SUCCESS;
}
