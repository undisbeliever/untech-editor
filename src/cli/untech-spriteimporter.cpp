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

    NamedList<SI::FrameSet> framesetContainer;

    for (int i = 1; i < argc; i++) {
        SI::Serializer::readFile(framesetContainer, argv[i]);
    }

    // ::DEBUG print all of the frames in the framesets::
    for (auto fs : framesetContainer) {
        std::cout << fs.first << ":\n";

        for (auto f : fs.second->frames()) {
            std::cout << "\t" << f.first << "\n";
        }
    }

    return EXIT_SUCCESS;
}
