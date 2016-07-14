#pragma once

#include "frameset.h"
#include "models/document.h"
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
    const static DocumentType DOCUMENT_TYPE;

public:
    MetaSpriteDocument();
    explicit MetaSpriteDocument(const std::string& filename);

    virtual ~MetaSpriteDocument() = default;

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
