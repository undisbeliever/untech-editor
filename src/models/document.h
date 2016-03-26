#ifndef _UNTECH_MODELS_DOCUMENT_H_
#define _UNTECH_MODELS_DOCUMENT_H_

#include "common/namedlist.h"
#include <string>

namespace UnTech {

/**
 * The Document abstract class is the root class of a document.
 *
 * Subclasses of document contains the list data stored in the document.
 *
 * Subclasses MUST load the document if the filename is specified in the
 * constructor.
 *
 * This is to ensure a seperation of the various model types while providing
 * a common pointer for extra functionality (ie, the Undo system).
 */
class Document {
public:
    Document() = default;
    Document(const std::string& filename)
        : _filename(filename)
    {
    }
    virtual ~Document() = default;

    std::string filename() const { return _filename; }
    void setFilename(const std::string& filename) { _filename = filename; }

    void writeFile(const std::string& filename)
    {
        writeDataFile(filename);
        _filename = filename;
    }

protected:
    virtual void writeDataFile(const std::string& filename) = 0;

private:
    std::string _filename;
};
}
#endif
