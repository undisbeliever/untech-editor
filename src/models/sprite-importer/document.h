#pragma once

#include "frameset.h"
#include "serializer.h"
#include "../document.h"
#include <string>

namespace UnTech {
namespace SpriteImporter {

class FrameSet;

/**
 * The root object of a SpriteImporter data structure.
 *
 * MEMORY: all children will be deleted when the document is deleted.
 */
class SpriteImporterDocument : public ::UnTech::Document {
public:
    SpriteImporterDocument()
        : Document()
        , _frameSet(*this)
    {
    }

    explicit SpriteImporterDocument(const std::string& filename)
        : Document(filename)
        , _frameSet(*this)
    {
        Serializer::readFile(_frameSet, filename);
    }

    virtual ~SpriteImporterDocument() = default;

    FrameSet& frameSet() { return _frameSet; }
    const FrameSet& frameSet() const { return _frameSet; }

    virtual void writeDataFile(const std::string& filename) override
    {
        Serializer::writeFile(_frameSet, filename);
        setFilename(filename);
    }

private:
    FrameSet _frameSet;
};
}
}
