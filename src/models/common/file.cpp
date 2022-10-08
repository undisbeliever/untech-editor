/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "file.h"
#include "string.h"
#include "models/common/exceptions.h"
#include "models/common/stringbuilder.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>

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

namespace UnTech::File {

std::vector<uint8_t> readBinaryFile(const std::filesystem::path& filePath, size_t limit)
{
    std::ifstream in(filePath, std::ios::in | std::ios::binary);
    if (!in) {
        throw runtime_error(u8"Cannot open file: ", filePath.u8string());
    }

    in.seekg(0, std::ios::end);
    auto pos = in.tellg();
    in.seekg(0);

    if (pos < 0) {
        throw runtime_error(u8"Cannot open file: ", filePath.u8string(), u8" : Cannot read file size");
    }
    const size_t size = pos;

    if (size > limit) {
        throw runtime_error(u8"Cannot open file : ", filePath.u8string(), u8" : file too large");
    }

    std::vector<uint8_t> ret(size);

    static_assert(sizeof(uint8_t) == sizeof(char));
    in.read(reinterpret_cast<char*>(ret.data()), ret.size());

    in.close();

    if (!in) {
        throw runtime_error(u8"Error reading file: ", filePath.u8string());
    }

    return ret;
}

std::u8string readUtf8TextFile(const std::filesystem::path& filePath)
{
    constexpr static unsigned N_BOM_CHARS = 3;
    constexpr static std::array<char8_t, 4> BOM{ 0xEF, 0xBB, 0xBF };
    static_assert(BOM.size() == sizeof(uint32_t));

    std::ifstream in(filePath, std::ios::in | std::ios::binary);
    if (!in) {
        throw runtime_error(u8"Cannot open file: ", filePath.u8string());
    }

    std::array<char8_t, 4> header{};
    static_assert(header.size() > N_BOM_CHARS);

    in.read(reinterpret_cast<char*>(header.data()), N_BOM_CHARS);
    assert(header.back() == 0);

    in.seekg(0, std::ios::end);
    auto size = in.tellg();

    if (size < 0) {
        throw runtime_error(u8"Cannot open file: ", filePath.u8string(), u8" : Cannot read file size");
    }
    if (size > 25 * 1024 * 1024) {
        throw runtime_error(u8"Cannot open file: ", filePath.u8string(), u8" : too large");
    }

    // check for BOM
    if (size >= N_BOM_CHARS && header == BOM) {
        in.seekg(3, std::ios::beg);
        size -= 3;
    }
    else {
        in.seekg(0, std::ios::beg);
    }

    std::u8string ret;
    if (size > 0) {
        ret.resize(size_t(size));

        in.read(reinterpret_cast<char*>(ret.data()), size);

        const auto read = in.gcount();
        const bool atEof = in.get() == decltype(in)::traits_type::eof();

        in.close();

        if (read != size) {
            throw runtime_error(u8"Error reading file: ", filePath.u8string(), u8" : Expected ", ptrdiff_t(size), u8" bytes got ", read, u8".");
        }
        if (atEof == false) {
            throw runtime_error(u8"Error reading file: ", filePath.u8string(), u8" : Not at end of file");
        }

        if (!String::checkUtf8WellFormed(ret)) {
            throw runtime_error(u8"Error reading file: ", filePath.u8string(), u8" : Not UTF-8 Well Formed");
        }
    }

    return (ret);
}

constexpr size_t MAX_ATOMIC_WRITE_SIZE = 256 * 1024 * 1024;

#ifdef PLATFORM_WINDOWS
void atomicWrite(const std::filesystem::path& filePath, std::span<const std::byte> data)
{
    const size_t BLOCK_SIZE = 4096;

    if (data.size() > MAX_ATOMIC_WRITE_SIZE) {
        throw runtime_error(u8"Cannot save file: data is too large");
    }

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
            throw runtime_error(u8"Temporary file already exists: ", tmpFilename.u8string());
        }
        else {
            throw runtime_error(u8"Cannot open temporary file ", tmpFilename.u8string());
        }
    }

    std::span<const std::byte> remaining = data;

    while (!remaining.empty()) {
        const DWORD toWrite = std::min(BLOCK_SIZE, remaining.size());
        DWORD written = 0;

        auto ret = WriteFile(hFile, remaining.data(), toWrite, &written, NULL);

        if (ret == FALSE || written != toWrite) {
            CloseHandle(hFile);
            throw runtime_error(u8"Error writing file: ", tmpFilename.u8string());
        }
        static_assert(std::is_unsigned_v<decltype(written)>);
        assert(written <= remaining.size());

        remaining = remaining.subspan(written);
    }

    CloseHandle(hFile);

    bool s = MoveFileExW(tmpFilename.c_str(), filePath.c_str(),
                         MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED);
    if (!s) {
        throw runtime_error(u8"MoveFileEx failed.  Cannot rename '", tmpFilename.u8string(), u8"' to '", filePath.u8string(), u8"'");
    }
}
#else
void atomicWrite(const std::filesystem::path& filePath, std::span<const std::byte> data)
{
    using namespace std::string_literals;

    const size_t BLOCK_SIZE = 4096;

    if (data.size() > MAX_ATOMIC_WRITE_SIZE) {
        throw runtime_error(u8"Cannot save file: data is too large");
    }

    // Temporary file will only be created if it does not exist.
    const std::filesystem::path tmpFilename = std::filesystem::path(filePath).concat("~");

    int mode = 0666;

    // Using lstat to detect if the filename is a symbolic link
    struct stat statbuf;
    if (::lstat(filePath.c_str(), &statbuf) == 0) {
        if (S_ISLNK(statbuf.st_mode)) {
            throw runtime_error(u8"Cannot write to a symbolic link: ", filePath.u8string());
        }

        // check if we can write to file
        bool canWrite = ((getuid() == statbuf.st_uid) && (statbuf.st_mode & S_IWUSR))
                        || ((getgid() == statbuf.st_gid) && (statbuf.st_mode & S_IWGRP))
                        || (statbuf.st_mode & S_IWOTH);
        if (!canWrite) {
            throw runtime_error(u8"User can not write to ", filePath.u8string());
        }

        mode = statbuf.st_mode & 0777;
    }

    const auto fd = ::open(tmpFilename.c_str(), O_EXCL | O_CREAT | O_WRONLY | O_NOFOLLOW, mode);
    if (fd < 0) {
        throw std::system_error(errno, std::system_category(),
                                "Cannot open file: " + tmpFilename.string());
    }

    std::span<const std::byte> remaining = data;

    while (!remaining.empty()) {
        const auto bytesToWrite = std::min(BLOCK_SIZE, remaining.size());

        const auto done = ::write(fd, remaining.data(), bytesToWrite);
        if (done < 0) {
            auto err = errno;
            ::close(fd);
            throw std::system_error(err, std::system_category(),
                                    "Cannot write to file: " + tmpFilename.string());
        }
        assert(done >= 0);
        assert(size_t(done) <= remaining.size());

        remaining = remaining.subspan(done);
    }

    int r;
    r = ::close(fd);
    if (r != 0) {
        throw std::system_error(errno, std::system_category(),
                                "Cannot close file: " + tmpFilename.string());
    }

    r = ::rename(tmpFilename.c_str(), filePath.c_str());
    if (r != 0) {
        throw std::system_error(errno, std::system_category(),
                                "Cannot rename '" + tmpFilename.string() + "' to '" + filePath.string() + "'");
    }
}
#endif

void atomicWrite(const std::filesystem::path& filePath, const std::vector<uint8_t>& data)
{
    atomicWrite(filePath, std::as_bytes(std::span{ data }));
}

void atomicWrite(const std::filesystem::path& filePath, const std::u8string& data)
{
    atomicWrite(filePath, std::as_bytes(std::span{ data }));
}

void atomicWrite(const std::filesystem::path& filePath, const std::u8string_view data)
{
    atomicWrite(filePath, std::as_bytes(std::span{ data }));
}

}
