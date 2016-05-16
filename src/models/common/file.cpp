#include "file.h"
#include "string.h"
#include <cstdio>
#include <string>
#include <fstream>
#include <stdexcept>

// ::TODO compile in windows::

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define PLATFORM_WINDOWS
#endif

#ifdef PLAYFORM_WINDOWS
#include <direct.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

#ifdef PLATFORM_WINDOWS
// This is the only module that requires windows character conversion

std::string to_string(const wchar_t* str = L"")
{
    if (!str) {
        str = L"";
    }
    auto length = WideCharToMultiByte(CP_UTF8, 0, s, -1, nullptr, 0, nullptr, nullptr);
    char[length + 1] buffer;
    WideCharToMultiByte(CP_UTF8, 0, s, -1, buffer, length, nullptr, nullptr);

    return std::string(buffer);
}

// use unique_ptr to prevent memory leak with exceptions.
inline std::unique_ptr<wchar_t*> to_wchar(const std::string str)
{
    auto length = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    auto buffer = new wchar_t[length + 1]();
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buffer, length);

    return std::unique_ptr<wchar_t*>(buffer);
}
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

std::string File::cwd()
{
#ifdef PLATFORM_WINDOWS
    wchar_t wpath[PATH_MAX + 1] = L"";

    if (!_wgetcwd(wpath, PATH_MAX)) {
        throw std::runtime_error("Error getting working directory");
    }

    return to_string(wpath);

#else
    char* dir = get_current_dir_name();

    if (dir) {
        std::string ret(dir);
        free(dir);
        return ret;
    }
    else {
        throw std::runtime_error("Error with path");
    }
#endif
}

std::string File::cleanPath(const std::string& path)
{
    if (path.empty()) {
        return path;
    }

#ifdef PLATFORM_WINDOWS
    const char SEP = '\\';

    std::string newPath = path;
    std::replace(newPath.begin(), newPath.end(), '/', '\\');

    const char* source = newPath.c_str();

#else
    const char SEP = '/';

    const char* source = path.c_str();
#endif

    char ret[path.length() + 1];

    char* pos = ret;
    char* dirs[path.length() / 2];
    size_t nDirs = 0;

    while (*source != 0) {
        // Start of a directory/file

        if (source[0] == '.' && source[1] == SEP) {
            // ignore all . directories
            source += 2;
        }

        else if (source[0] == '.' && source[1] == '.' && source[2] == SEP) {
            if (nDirs > 1) {
                // shift pos to previous directory

                pos = dirs[nDirs - 2];
                nDirs--;
                source += 3;
            }
            else {
                pos[0] = source[0];
                pos[1] = source[1];
                pos[2] = source[2];
                pos += 3;
                source += 3;
            }
        }

        else {
            // loop until end of directory separator

            while (*source != 0 && *source != SEP) {
                *(pos++) = *(source++);
            }

            // include terminator ('/' or '\0').
            *(pos++) = *(source++);

            if (*(source - 1) == 0) {
                break;
            }
            else if (*(source - 1) == SEP) {
                // remember previous dir position
                dirs[nDirs] = pos;
                nDirs++;
            }
        }

#ifndef PLATFORM_WINDOWS
        // ignore duplicate separators
        while (*source == SEP) {
            source++;
        }
#endif
    }

    return std::string(ret);
}

std::string File::joinPath(const std::string& dir, const std::string& path)
{
#ifdef PLATFORM_WINDOWS
    auto wdir = to_wchar(dir);
    auto wpath = to_wchar(path);
    wchar_t wjoined[PATH_MAX] = L"";

    PathCombineW(wjoined, wdir.get(), wpath.get());

    return to_string(wjoined);

#else
    if (path.front() == '/') {
        return path;
    }

    std::string join;
    join.reserve(dir.length() + path.length() + 1);

    if (!dir.empty() && dir.back() != '/') {
        join = dir + '/' + path;
    }
    else {
        join = dir + path;
    }

    return cleanPath(join);
#endif
}

std::string File::fullPath(const std::string& path)
{
#ifdef PLATFORM_WINDOWS
    wchar_t wfullpath[PATH_MAX] = L"";
    auto wpath = to_wchar(path);

    if (!_wfullpath(wfile_name, wpath.get(), PATH_MAX)) {
        throw std::runtime_error("Error expanding path");
    }

    return to_string(wfullpath);

#else
    if (path.front() == '/') {
        return path;
    }

    return joinPath(cwd(), path);
#endif
}

std::string File::relativePath(const std::string& sourceDir, const std::string& destPath)
{
#ifdef PLATFORM_WINDOWS
    wchar_t wrelpath[PATH_MAX] = L"";
    auto wsource = to_wchar(sourceDir);
    auto wdest = to_wchar(destPath);

    if (!PathRelativePathToW(wrelpath, wsource.get(), FILE_ATTRIBUTE_DIRECTORY, wdest.get(), FILE_ATTRIBUTE_NORMAL)) {
        throw std::runtime_error("Error expanding path");
    }

    return to_string(wrelpath);

#else
    std::string source = File::fullPath(sourceDir);
    std::string dest = File::fullPath(destPath);

    if (source.empty() || dest.empty()
        || source.front() != '/' || dest.front() != '/') {

        return dest;
    }

    std::string ret;

    // Get common root directories
    size_t lastCommonSep = 0;
    {
        size_t i = 0;
        while (i < source.size() && i < dest.size() && source[i] == dest[i]) {
            if (source[i] == '/') {
                lastCommonSep = i;
            }
            i++;
        }

        if (i == source.size() && dest.size() > source.size() && dest[i] == '/') {
            lastCommonSep = i;
        }
    }

    if (source.size() > lastCommonSep) {
        size_t p = lastCommonSep + 1;

        while ((p = source.find('/', p)) != std::string::npos) {
            ret += "../";
            p += 1;
        }
    }

    ret += dest.substr(lastCommonSep + 1);

    return ret;
#endif
}
