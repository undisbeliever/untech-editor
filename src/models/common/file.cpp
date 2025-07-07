/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "file.h"
#include "models/common/exceptions.h"
#include "models/common/string.h"
#include "models/common/stringbuilder.h"
#include "models/common/u8strings.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>

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
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
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

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
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

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
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

void writeFile(const std::filesystem::path& filePath, std::span<const std::byte> data)
{
    if (data.size() > MAX_ATOMIC_WRITE_SIZE) {
        throw runtime_error(u8"Cannot save file: data is too large");
    }

    auto f = std::ofstream(filePath, std::ios::out | std::ios::binary);

    if (!f) {
        throw runtime_error(u8"Cannot open file: ", filePath.u8string());
    }

    try {
        f.exceptions(std::ios_base::failbit | std::ios_base::badbit);
        f.write(reinterpret_cast<const char*>(data.data()), data.size());
        f.flush();
        f.close();
    }
    catch (const std::system_error& ex) {
        throw runtime_error(u8"Cannot write to ", filePath.u8string(), u8"\n", convert_old_string(ex.code().message()));
    }
}

void writeFile(const std::filesystem::path& filePath, const std::vector<uint8_t>& data)
{
    writeFile(filePath, std::as_bytes(std::span{ data }));
}

void writeFile(const std::filesystem::path& filePath, const std::u8string& data)
{
    writeFile(filePath, std::as_bytes(std::span{ data }));
}

void writeFile(const std::filesystem::path& filePath, const std::u8string_view data)
{
    writeFile(filePath, std::as_bytes(std::span{ data }));
}

}
