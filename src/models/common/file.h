/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <cstdint>
#include <string>
#include <utility>
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
std::vector<uint8_t> readBinaryFile(const std::string& filename, size_t limit);

/**
 * Reads a UTF-8 text file into a string.
 *
 * If the file has a UTF-8 BOM it will be removed.
 *
 * This function checks that the file is well formed.
 *
 * Raises an exception if an error occurred.
 */
std::string readUtf8TextFile(const std::string& filename);

/**
 * Splits a filename into its dir, pathname components.
 *
 * Dir is either blank or ends in a slash, making it simple for filename
 * concatenation.
 */
std::pair<std::string, std::string> splitFilename(const std::string& filename);

/**
 * Returns the current working directory
 */
std::string cwd();

/**
 * Cleans the path, removing the ./ and ../ directories
 */
std::string cleanPath(const std::string& path);

/**
 * Returns the joined path of the current path.
 */
std::string joinPath(const std::string& dir, const std::string& path);

/**
 * Returns the full path of the current path.
 *
 * This does not expand symlinks in linux
 */
std::string fullPath(const std::string& path);

/**
 * Returns a path to `destPath`, relative to `sourceDir`
 *
 * If `sourceDir` is empty then the full path of `destPath` is returned.
 */
std::string relativePath(const std::string& sourceDir, const std::string& destPath);

/**
 * Preforms an atomic file write of a block of memory.
 *
 * Will raise an exception if an error occurred.
 *
 * This function will leave a tempfile on the disk (not modifying the
 * original) if an error occurred while writing it.
 */
void atomicWrite(const std::string& filename, const void* data, size_t size);
void atomicWrite(const std::string& filename, const std::vector<uint8_t>& data);
}
}
