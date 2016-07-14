#pragma once

#include "common/file.h"
#include "common/namedlist.h"
#include <string>

namespace UnTech {

struct DocumentType {
    // MUST NOT CONTAIN PIPE CHARACTER
    const std::string name;

    // MUST NOT CONTAIN PIPE OR START
    const std::string extension;
};

/**
 * The Document abstract class is the root class of a document.
 *
 * Subclasses of document contains the list data stored in the document.
 *
 * Subclasses MUST load the document if the filename is specified in the
 * constructor.
 *
 * This is to ensure a separation of the various model types while providing
 * a common pointer for extra functionality (ie, the Undo system).
 */
class Document {
public:
    Document() = default;
    Document(const std::string& filename)
    {
        _filename = File::fullPath(filename);
    }

    virtual ~Document() = default;

    const std::string& filename() const { return _filename; }
    void setFilename(const std::string& filename)
    {
        _filename = File::fullPath(filename);
    }

    void save()
    {
        if (!_filename.empty()) {
            writeDataFile(_filename);
        }
    }

    void saveFile(const std::string& filename)
    {
        auto newFilename = File::fullPath(filename);

        writeDataFile(newFilename);

        _filename = newFilename;
    }

    virtual const DocumentType& documentType() const = 0;

protected:
    virtual void writeDataFile(const std::string& filename) = 0;

private:
    std::string _filename;
};
}
