#ifndef _UNTECH_MODELS_METASPRITE_DOCUMENT_H_
#define _UNTECH_MODELS_METASPRITE_DOCUMENT_H_

#include "frameset.h"
#include "serializer.h"
#include "../document.h"
#include <string>

namespace UnTech {
namespace MetaSprite {

class FrameSet;

class MetaSpriteDocument : public ::UnTech::Document {
public:
    MetaSpriteDocument()
        : Document()
        , _frameSet(std::make_shared<FrameSet>(*this))
    {
    }

    explicit MetaSpriteDocument(const std::string& filename)
        : Document(filename)
        , _frameSet(std::make_shared<FrameSet>(*this))
    {
        Serializer::readFile(_frameSet, filename);
    }

    virtual ~MetaSpriteDocument() = default;

    const std::shared_ptr<FrameSet>& frameSet() const { return _frameSet; }

    virtual void writeDataFile(const std::string& filename) override
    {
        Serializer::writeFile(*_frameSet, filename);
        setFilename(filename);
    }

private:
    std::shared_ptr<FrameSet> _frameSet;
};
}
}
#endif
