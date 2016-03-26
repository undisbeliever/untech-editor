#ifndef _UNTECH_MODELS_SPRITEIMPORTER_DOCUMENT_H_
#define _UNTECH_MODELS_SPRITEIMPORTER_DOCUMENT_H_

#include "frameset.h"
#include "serializer.h"
#include "../document.h"
#include "../common/namedlist.h"
#include <string>

namespace UnTech {
namespace SpriteImporter {

class SpriteImporterDocument : public ::UnTech::Document {
public:
    SpriteImporterDocument()
        : Document()
        , _siFramesets(*this)
    {
    }

    explicit SpriteImporterDocument(const std::string& filename)
        : Document(filename)
        , _siFramesets(*this)
    {
        Serializer::readFile(_siFramesets, filename);
    }

    virtual ~SpriteImporterDocument() = default;

    virtual void writeDataFile(const std::string& filename) override
    {
        Serializer::writeFile(_siFramesets, filename);
        setFilename(filename);
    }

    inline FrameSet::list_t& spriteImporterFramesets() { return _siFramesets; }

private:
    FrameSet::list_t _siFramesets;
};
}
}
#endif
