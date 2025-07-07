/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <filesystem>
#include <span>
#include <string>
#include <vector>

namespace UnTech::File {

/**
 * Reads a binary file into a vector.
 *
 * If the size of the file is greater than limit then a runtime_error is
 * thrown.
 *
 * Raises an exception if an error occurred.
 */
std::vector<uint8_t> readBinaryFile(const std::filesystem::path& filePath, size_t limit);

/**
 * Reads a UTF-8 text file into a string.
 *
 * If the file has a UTF-8 BOM it will be removed.
 *
 * This function checks that the file is well formed.
 *
 * Raises an exception if an error occurred.
 */
std::u8string readUtf8TextFile(const std::filesystem::path& filePath);

/**
 * Writes `data` to a file on disk using `std::ofstream`.
 *
 * Will raise an exception if an error occurred.
 */
void writeFile(const std::filesystem::path& filePath, std::span<const std::byte> data);
void writeFile(const std::filesystem::path& filePath, const std::vector<uint8_t>& data);
void writeFile(const std::filesystem::path& filePath, const std::u8string& data);
void writeFile(const std::filesystem::path& filePath, const std::u8string_view data);

}
