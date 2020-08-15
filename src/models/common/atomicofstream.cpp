/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "atomicofstream.h"
#include "file.h"
#include <stdexcept>

using namespace UnTech;

AtomicOfStream::AtomicOfStream(const std::filesystem::path& filePath, ios_base::openmode mode)
    : std::ostringstream(mode)
    , _filePath(std::filesystem::absolute(filePath))
    , _state(State::WRITING)
{
    if (filePath.empty()) {
        throw std::invalid_argument("Empty filename");
    }
}

// Must put destructor here to prevent a
// "multiple definition of `vtable for UnTech::AtomicOfStream'"
// compile error with `x86_64-w64-mingw32-g++ (GCC) 10.2.0`
AtomicOfStream::~AtomicOfStream() = default;

void AtomicOfStream::abort()
{
    this->str("");
    _state = State::ABORTED;
}

void AtomicOfStream::commit()
{
    if (_state != State::WRITING) {
        return;
    }

    if (this->fail()) {
        return;
    }

    const std::string& str = this->str();
    File::atomicWrite(_filePath, str.c_str(), str.size());

    _state = State::COMMITTED;
}
