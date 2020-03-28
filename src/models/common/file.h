/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace UnTech {
namespace File {

/**
 * Reads a binary file into a vector.
 *
 * If the size of the file is greater than limit then a runtime_error is
 * thrown.
 *
 * Raises an exception if an error occurred.
 */
std::vector<uint8_t> readBinaryFile(const std::filesystem::path& filename, size_t limit);

/**
 * Reads a UTF-8 text file into a string.
 *
 * If the file has a UTF-8 BOM it will be removed.
 *
 * This function checks that the file is well formed.
 *
 * Raises an exception if an error occurred.
 */
std::string readUtf8TextFile(const std::filesystem::path& filePath);

/**
 * Preforms an atomic file write of a block of memory.
 *
 * Will raise an exception if an error occurred.
 *
 * This function will leave a tempfile on the disk (not modifying the
 * original) if an error occurred while writing it.
 */
void atomicWrite(const std::filesystem::path& filePath, const void* data, size_t size);
void atomicWrite(const std::filesystem::path& filePath, const std::vector<uint8_t>& data);
void atomicWrite(const std::filesystem::path& filePath, const std::string& data);
}
}
