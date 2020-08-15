/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "file.h"
#include "string.h"
#include "models/common/stringbuilder.h"
#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
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

using namespace UnTech;

std::vector<uint8_t> File::readBinaryFile(const std::filesystem::path& filePath, size_t limit)
{
    std::ifstream in(filePath, std::ios::in | std::ios::binary);
    if (!in) {
        throw std::runtime_error(stringBuilder("Cannot open file: ", filePath.u8string()));
    }

    in.seekg(0, std::ios::end);
    auto pos = in.tellg();
    in.seekg(0);

    if (pos < 0) {
        throw std::runtime_error(stringBuilder("Cannot open file: ", filePath.u8string(), " : Cannot read file size"));
    }
    const size_t size = pos;

    if (size > limit) {
        throw std::runtime_error(stringBuilder("Cannot open file : ", filePath.u8string(), " : file too large"));
    }

    std::vector<uint8_t> ret(size);
    in.read((char*)ret.data(), ret.size());
    in.close();

    if (!in) {
        throw std::runtime_error(stringBuilder("Error reading file: ", filePath.u8string()));
    }

    return ret;
}

std::string File::readUtf8TextFile(const std::filesystem::path& filePath)
{
    constexpr static unsigned N_BOM_CHARS = 3;
    constexpr static std::array<uint8_t, 4> BOM{ 0xEF, 0xBB, 0xBF };
    static_assert(BOM.size() == sizeof(uint32_t));

    std::ifstream in(filePath, std::ios::in | std::ios::binary);
    if (!in) {
        throw std::runtime_error(stringBuilder("Cannot open file: ", filePath.u8string()));
    }

    std::array<uint8_t, 4> header{};
    static_assert(header.size() > N_BOM_CHARS);

    in.read((char*)header.data(), N_BOM_CHARS);
    assert(header.back() == 0);

    in.seekg(0, std::ios::end);
    auto size = in.tellg();

    if (size < 0) {
        throw std::runtime_error(stringBuilder("Cannot open file: ", filePath.u8string(), " : Cannot read file size"));
    }
    if (size > 25 * 1024 * 1024) {
        throw std::runtime_error(stringBuilder("Cannot open file: ", filePath.u8string(), " : too large"));
    }

    // check for BOM
    if (size >= N_BOM_CHARS && header == BOM) {
        in.seekg(3, std::ios::beg);
        size -= 3;
    }
    else {
        in.seekg(0, std::ios::beg);
    }

    std::string ret;
    if (size > 0) {
        ret.resize(size_t(size));

        in.read(ret.data(), size);

        const auto read = in.gcount();
        const bool atEof = in.get() == decltype(in)::traits_type::eof();

        in.close();

        if (read != size) {
            throw std::runtime_error(stringBuilder("Error reading file: ", filePath.u8string(), " : Expected ", ptrdiff_t(size), " bytes got ", read, "."));
        }
        if (atEof == false) {
            throw std::runtime_error(stringBuilder("Error reading file: ", filePath.u8string(), " : Not at end of file"));
        }

        if (!String::checkUtf8WellFormed(ret)) {
            throw std::runtime_error(stringBuilder("Error reading file: ", filePath.u8string(), " : Not UTF-8 Well Formed"));
        }
    }

    return (ret);
}

void File::atomicWrite(const std::filesystem::path& filePath, const std::vector<uint8_t>& data)
{
    atomicWrite(filePath, data.data(), data.size());
}

void File::atomicWrite(const std::filesystem::path& filePath, const std::string& data)
{
    atomicWrite(filePath, data.data(), data.size());
}

// ::TODO replace with std::span when upgrading to c++20::
#ifdef PLATFORM_WINDOWS
void File::atomicWrite(const std::filesystem::path& filePath, const void* data, size_t size)
{
    const size_t BLOCK_SIZE = 4096;

    // ::TODO properly test this function::

    // I could not find a function to create and open a temporary file in a given directory in windows.
    // Instead we create a `~` temp file and throw an error if tmpFilename already exists.
    const std::filesystem::path tmpFilename = std::filesystem::path(filePath).concat(L"~");

    // open a new file for writing.
    // error if file exists.
    HANDLE hFile = CreateFileW(tmpFilename.c_str(), GENERIC_WRITE, 0, nullptr,
                               CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hFile == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_EXISTS) {
            throw std::runtime_error(stringBuilder("Temporary file already exists: ", tmpFilename.string()));
        }
        else {
            throw std::runtime_error(stringBuilder("Cannot open temporary file ", tmpFilename.string()));
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

    bool s = MoveFileExW(tmpFilename.c_str(), filePath.c_str(),
                         MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED);
    if (!s) {
        throw std::runtime_error("MoveFileEx failed");
    }
}
#else
void File::atomicWrite(const std::filesystem::path& filePath, const void* data, size_t size)
{
    using namespace std::string_literals;

    const std::string filename = filePath.u8string();

    const size_t BLOCK_SIZE = 4096;

    std::string tmpFilename = filePath.native() + "-tmpXXXXXX"s;

    int fd;
    struct stat statbuf;

    // Using lstat to detect if the filename is a symbolic link
    if (lstat(filename.c_str(), &statbuf) == 0) {
        if (S_ISLNK(statbuf.st_mode)) {
            throw std::runtime_error("Cannot write to a symbolic link");
        }

        // check if we can write to file
        bool canWrite = ((getuid() == statbuf.st_uid) && (statbuf.st_mode & S_IWUSR))
                        || ((getgid() == statbuf.st_gid) && (statbuf.st_mode & S_IWGRP))
                        || (statbuf.st_mode & S_IWOTH);
        if (!canWrite) {
            throw std::runtime_error(stringBuilder("User can not write to ", filename));
        }

        fd = mkstemp(tmpFilename.data());
        if (fd < 0) {
            throw std::system_error(errno, std::system_category());
        }

        // mkstemp sets file permission to 0600
        // Set the permissions to match filename
        int s = 0;
        s |= chmod(tmpFilename.c_str(), statbuf.st_mode);
        s |= chown(tmpFilename.c_str(), statbuf.st_uid, statbuf.st_gid);
        if (s != 0) {
            std::cerr << "Warning: unable to change the file permissions of " << tmpFilename << std::endl;
        }
    }
    else {
        fd = mkstemp(tmpFilename.data());
        if (fd < 0) {
            throw std::system_error(errno, std::system_category());
        }

        // mkostemp sets file permission to 0600
        // Set the permissions what user would expect
        mode_t mask = umask(0);
        umask(mask);
        chmod(tmpFilename.c_str(), 0666 & ~mask);
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

    r = rename(tmpFilename.c_str(), filename.c_str());
    if (r != 0) {
        throw std::system_error(errno, std::system_category());
    }
}
#endif
