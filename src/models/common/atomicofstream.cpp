#include "atomicofstream.h"
#include "file.h"
#include <stdexcept>

using namespace UnTech;

AtomicOfStream::AtomicOfStream(const std::string& filename, ios_base::openmode mode)
    : std::ostringstream(mode)
    , _filename(File::fullPath(filename))
    , _state(State::WRITING)
{
    if (filename.empty()) {
        throw std::invalid_argument("Empty filename");
    }
}

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
    File::atomicWrite(_filename, str.c_str(), str.size());

    _state = State::COMMITTED;
}
