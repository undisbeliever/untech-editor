#include "atomicofstream.h"
#include "file.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

// ::TODO windows equivalent::
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>

using namespace UnTech;

inline std::string makeTempFilename(const std::string& filename)
{
    char tmp[filename.size() + 12];
    strcpy(tmp, filename.c_str());
    strcat(tmp, ".XXXXXX");

    if (mktemp(tmp) == nullptr) {
        throw std::runtime_error("Could not make a tempfile");
    }

    return std::string(tmp);
}

AtomicOfStream::AtomicOfStream(const std::string& filename,
                               std::ios_base::openmode mode)
    : _filename(File::fullPath(filename))
    , _tmpFilename(makeTempFilename(_filename))
    , _state(State::WRITING)
{
    struct stat statbuf;

    if (stat(_filename.c_str(), &statbuf) == 0) {

        if (S_ISLNK(statbuf.st_mode)) {
            throw std::runtime_error("Cannot write to a symbolic link");
        }

        // check if can write to file
        bool canWrite = ((getuid() == statbuf.st_uid) && (statbuf.st_mode & S_IWUSR))
                        || ((getgid() == statbuf.st_gid) && (statbuf.st_mode & S_IWGRP))
                        || (statbuf.st_mode & S_IWOTH);

        if (!canWrite) {
            throw std::runtime_error("User can not write to " + _filename);
        }

        // open file
        std::ofstream::open(_tmpFilename, mode);

        // change permissions to match filename
        // This may fail, do it silently
        chmod(_tmpFilename.c_str(), statbuf.st_mode);
        chown(_tmpFilename.c_str(), statbuf.st_uid, statbuf.st_gid);
    }
    else {
        std::ofstream::open(_tmpFilename, mode);
    }
}

void AtomicOfStream::close()
{
    std::ofstream::close();
    _state = State::ABORTED;
}

void AtomicOfStream::abort()
{
    std::ofstream::close();

    if (_state == State::WRITING) {
        auto ret = remove(_tmpFilename.c_str());
        if (ret != 0) {
            throw std::runtime_error(std::string("Could not remove file: ")
                                     + std::strerror(errno));
        }

        _state = State::ABORTED;
    }
}

void AtomicOfStream::commit()
{
    if (_state == State::WRITING) {
        std::ofstream::close();

        if (!this->fail()) {
            auto ret = rename(_tmpFilename.c_str(), _filename.c_str());
            if (ret != 0) {
                throw std::runtime_error(std::string("Could not rename file: ")
                                         + std::strerror(errno));
            }
        }
        _state = State::COMMITTED;
    }
}
