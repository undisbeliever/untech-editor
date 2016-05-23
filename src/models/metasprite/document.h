#pragma once

#include "frameset.h"
#include "serializer.h"
#include "../document.h"
#include <string>

namespace UnTech {
namespace MetaSprite {

/**
 * The root object of a MetaSprite data structure.
 *
 * MEMORY: all children will be deleted when the document is deleted.
 */
class MetaSpriteDocument : public ::UnTech::Document {
public:
    MetaSpriteDocument()
        : Document()
        , _frameSet(*this)
    {
    }

    explicit MetaSpriteDocument(const std::string& filename)
        : Document(filename)
        , _frameSet(*this)
    {
        Serializer::readFile(_frameSet, filename);
    }

    virtual ~MetaSpriteDocument() = default;

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
