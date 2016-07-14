#pragma once

#include "frameset.h"
#include "models/document.h"
#include <string>

namespace UnTech {
namespace SpriteImporter {

/**
 * The root object of a SpriteImporter data structure.
 *
 * MEMORY: all children will be deleted when the document is deleted.
 */
class SpriteImporterDocument : public ::UnTech::Document {
public:
    const static DocumentType DOCUMENT_TYPE;

public:
    SpriteImporterDocument();
    explicit SpriteImporterDocument(const std::string& filename);

    virtual ~SpriteImporterDocument() = default;

    FrameSet& frameSet() { return _frameSet; }
    const FrameSet& frameSet() const { return _frameSet; }

    virtual const DocumentType& documentType() const override;

protected:
    virtual void writeDataFile(const std::string& filename) override;

private:
    FrameSet _frameSet;
};
}
}
