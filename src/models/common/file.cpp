#include "file.h"
#include "string.h"
#include <string>
#include <fstream>

using namespace UnTech;
using namespace UnTech::File;

std::string readUtf8TextFile(const std::string& filename)
{
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (in) {
        char bom[3];
        in.read(bom, sizeof(bom));

        in.seekg(0, std::ios::end);
        size_t size = in.tellg();

        // check for BOM
        if (size > 3 && bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF) {
            in.seekg(3, std::ios::beg);
            size -= 3;
        }
        else {
            in.seekg(0, std::ios::beg);
        }

        std::string ret;
        ret.reserve(size + 1);

        ret.assign((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>()));

        in.close();

        if (!String::checkUtf8WellFormed(ret)) {
            throw("File is not UTF-8 Well Formed");
        }

        return (ret);
    }
    throw("Cannot open file");
}
