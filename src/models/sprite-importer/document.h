#ifndef _UNTECH_MODELS_SPRITEIMPORTER_DOCUMENT_H_
#define _UNTECH_MODELS_SPRITEIMPORTER_DOCUMENT_H_

#include "serializer.h"
#include "../document.h"
#include "frameset.h"
#include <string>

namespace UnTech {
namespace SpriteImporter {

class FrameSet;

class SpriteImporterDocument : public ::UnTech::Document {
public:
    SpriteImporterDocument()
        : Document()
        , _frameSet(std::make_shared<FrameSet>(*this))
    {
    }

    explicit SpriteImporterDocument(const std::string& filename)
        : Document(filename)
        , _frameSet(std::make_shared<FrameSet>(*this))
    {
        Serializer::readFile(_frameSet, filename);
    }

    virtual ~SpriteImporterDocument() = default;

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
