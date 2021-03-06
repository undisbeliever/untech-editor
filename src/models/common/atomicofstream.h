/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <filesystem>
#include <sstream>

namespace UnTech {

/**
 * AtomicOfStream provides a **BASIC** atomicity to file writing.
 *
 * The class will write to a memory instead of disk and upon commit will
 * write to disk though a temporary file.
 *
 * NOTE: Checking that the filename is valid is only preformed on
 * `commit()`.
 */
class AtomicOfStream : public std::ostringstream {
public:
    AtomicOfStream() = delete;
    AtomicOfStream(const AtomicOfStream&) = delete;
    AtomicOfStream(const AtomicOfStream&&) = delete;

    AtomicOfStream(const std::filesystem::path& filePath,
                   ios_base::openmode mode = ios_base::out | ios_base::binary);

    virtual ~AtomicOfStream() override;

    /** Closes the stream and does not save to disk */
    void abort();

    /**
     * Writes the stream to the filename and closes the stream.
     * Raises an exception if an error occurred when writing the file.
     */
    void commit();

private:
    enum State {
        WRITING,
        ABORTED,
        COMMITTED
    };

private:
    const std::filesystem::path _filePath;
    State _state;
};
}
