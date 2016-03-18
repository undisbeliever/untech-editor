#include "file.h"
#include "string.h"
#include <string>
#include <fstream>
#include <stdexcept>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define PLATFORM_WINDOWS
#endif

using namespace UnTech;

std::string File::readUtf8TextFile(const std::string& filename)
{
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (in) {
        uint8_t bom[3];
        in.read((char*)bom, sizeof(bom));

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
            throw std::runtime_error("File is not UTF-8 Well Formed");
        }

        return (ret);
    }
    throw std::runtime_error("Cannot open file");
}

std::pair<std::string, std::string> File::splitFilename(const std::string& filename)
{
    if (filename.empty()) {
        return { std::string(), std::string() };
    }

#ifdef PLATFORM_WINDOWS
    // search both types of slash in windows.
    auto i = filename.find_last_of("\\/");
#else
    auto i = filename.find_last_of('/');
#endif

    if (i == std::string::npos) {
        return { std::string(), filename };
    }
    else {
        return { filename.substr(0, i + 1), filename.substr(i + 1) };
    }
}
