#include "file.h"
#include "string.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define PLATFORM_WINDOWS
#endif

#ifdef PLATFORM_WINDOWS
#include <direct.h>
#include <shlwapi.h>
#include <windows.h>
#else
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#ifdef PLATFORM_WINDOWS
// This is the only module that requires windows character conversion

std::string to_string(const wchar_t* str = L"")
{
    if (!str) {
        str = L"";
    }
    auto length = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
    std::unique_ptr<char[]> buffer(new char[length + 1]);
    WideCharToMultiByte(CP_UTF8, 0, str, -1, buffer.get(), length, nullptr, nullptr);

    return std::string(buffer.get());
}

// use unique_ptr to prevent memory leak with exceptions.
inline std::unique_ptr<wchar_t[]> to_wchar(const std::string str)
{
    auto length = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::unique_ptr<wchar_t[]> buffer(new wchar_t[length + 1]);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buffer.get(), length);

    return std::move(buffer);
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
    wchar_t wpath[PATH_MAX] = L"";

    if (!_wgetcwd(wpath, PATH_MAX)) {
        throw std::runtime_error("Error getting working directory");
    }

    return to_string(wpath);

#else
    char buff[PATH_MAX + 1];
    char* dir = getcwd(buff, PATH_MAX);

    if (dir) {
        std::string ret(dir);
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

    while (*source != '\0') {
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

            while (*source != '\0' && *source != SEP) {
                *(pos++) = *(source++);
            }

            // include terminator ('/' or '\0').
            *(pos++) = *(source++);

            if (*(source - 1) == '\0') {
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

    if (!_wfullpath(wfullpath, wpath.get(), PATH_MAX)) {
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
    if (sourceDir.empty()) {
        return File::fullPath(destPath);
    }

#ifdef PLATFORM_WINDOWS
    wchar_t wrelpath[PATH_MAX] = L"";
    auto wsource = to_wchar(sourceDir);
    auto wdest = to_wchar(destPath);

    if (!PathRelativePathToW(wrelpath, wsource.get(), FILE_ATTRIBUTE_DIRECTORY, wdest.get(), FILE_ATTRIBUTE_NORMAL)) {
        return File::fullPath(destPath);
    }

    if (wrelpath[0] != L'\\') {
        return to_string(wrelpath);
    }
    else {
        return to_string(wrelpath + 1);
    }

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

#ifdef PLATFORM_WINDOWS
void File::atomicWrite(const std::string& filename, const void* data, size_t size)
{
    const size_t BLOCK_SIZE = 4096;

    // atomically creating a tempfile name is hard
    // just simplify it by using a ~ temp file instead.
    auto tmpFilename = to_wchar(filename + "~");

    // open file normally.
    // error out if file exists.
    HANDLE hFile = CreateFileW(tmpFilename.get(), GENERIC_WRITE, 0, nullptr,
                               CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hFile == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_EXISTS) {
            throw std::runtime_error("Temporary file already exists: " + filename + "~");
        }
        else {
            throw std::runtime_error("Cannot open temporary file " + filename + "~");
        }
    }

    const uint8_t* ptr = static_cast<const uint8_t*>(data);
    size_t todo = size;

    while (todo > 0) {
        DWORD toWrite = std::min(BLOCK_SIZE, todo);
        DWORD written = 0;

        auto ret = WriteFile(hFile, ptr, toWrite, &written, NULL);

        if (ret == FALSE || written != toWrite) {
            CloseHandle(hFile);
            throw std::runtime_error("Error writing file");
        }

        ptr += toWrite;
        todo -= toWrite;
    }

    CloseHandle(hFile);

    auto wFilename = to_wchar(filename);
    bool s = MoveFileExW(tmpFilename.get(), wFilename.get(),
                         MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED);
    if (!s) {
        throw std::runtime_error("MoveFileEx failed");
    }
}
#else
void File::atomicWrite(const std::string& filename, const void* data, size_t size)
{
    const size_t BLOCK_SIZE = 4096;

    int fd;
    char tmpFilename[filename.size() + 12];
    strncpy(tmpFilename, filename.c_str(), filename.size() + 1);
    strncpy(tmpFilename + filename.size(), "-tmpXXXXXX", 12);

    struct stat statbuf;
    if (stat(filename.c_str(), &statbuf) == 0) {
        if (S_ISLNK(statbuf.st_mode)) {
            throw std::runtime_error("Cannot write to a symbolic link");
        }

        // check if we can write to file
        bool canWrite = ((getuid() == statbuf.st_uid) && (statbuf.st_mode & S_IWUSR))
                        || ((getgid() == statbuf.st_gid) && (statbuf.st_mode & S_IWGRP))
                        || (statbuf.st_mode & S_IWOTH);
        if (!canWrite) {
            throw std::runtime_error("User can not write to " + filename);
        }

        fd = mkostemp(tmpFilename, O_WRONLY);
        if (fd < 0) {
            throw std::system_error(errno, std::system_category());
        }

        // mkostemp sets file permission to 0600
        // Set the permissions to match filename
        chmod(tmpFilename, statbuf.st_mode);
        chown(tmpFilename, statbuf.st_uid, statbuf.st_gid);
    }
    else {
        fd = mkostemp(tmpFilename, O_WRONLY);
        if (fd < 0) {
            throw std::system_error(errno, std::system_category());
        }

        // mkostemp sets file permission to 0600
        // Set the permissions what user would expect
        mode_t mask = umask(0);
        umask(mask);
        chmod(tmpFilename, 0666 & ~mask);
    }

    const uint8_t* ptr = static_cast<const uint8_t*>(data);
    size_t todo = size;
    ssize_t done;

    while (todo > 0) {
        done = ::write(fd, ptr, std::min(BLOCK_SIZE, todo));
        if (done < 0) {
            auto err = errno;
            close(fd);
            throw std::system_error(err, std::system_category());
        }

        ptr += done;
        todo -= done;
    }

    int r;
    r = close(fd);
    if (r != 0) {
        throw std::system_error(errno, std::system_category());
    }

    r = rename(tmpFilename, filename.c_str());
    if (r != 0) {
        throw std::system_error(errno, std::system_category());
    }
}
#endif
