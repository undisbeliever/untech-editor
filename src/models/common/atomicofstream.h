#pragma once

#include <fstream>
#include <string>

namespace UnTech {

/**
 * AtomicOfStream provides a **BASIC** atomicity to a std::ofstream
 *
 * The class will write to a temporary file instead of filename
 * and reform a rename of the tmpfile to the filename on commit().
 *
 * Users must NOT call std::ofstream::open on this class.
 */
class AtomicOfStream : public std::ofstream {
public:
    AtomicOfStream() = delete;
    AtomicOfStream(const AtomicOfStream&) = delete;
    AtomicOfStream(const AtomicOfStream&&) = delete;

    AtomicOfStream(const std::string& filename,
                   std::ios_base::openmode mode = std::ios_base::out);

    virtual ~AtomicOfStream() {}

    /** closes the file but does not remove it */
    void close();

    /** Closes and removes the tmpfile */
    void abort();

    /** Closes the file and renames it to filename. */
    void commit();

    inline const std::string& tmpFilename() const { return _tmpFilename; }

private:
    enum State {
        WRITING,
        ABORTED,
        COMMITTED
    };

private:
    const std::string _filename, _tmpFilename;
    State _state;
};
}
