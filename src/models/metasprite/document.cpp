#include "document.h"

#include "serializer.h"
#include "models/metasprite.h"

using namespace UnTech::MetaSprite;

const UnTech::DocumentType MetaSpriteDocument::DOCUMENT_TYPE = {
    "UnTech MetaSprite Document",
    "utms"
};

MetaSpriteDocument::MetaSpriteDocument()
    : Document()
    , _frameSet(*this)
{
}

MetaSpriteDocument::MetaSpriteDocument(const std::string& filename)
    : Document(filename)
    , _frameSet(*this)
{
    Serializer::readFile(_frameSet, filename);
}

void MetaSpriteDocument::writeDataFile(const std::string& filename)
{
    Serializer::writeFile(_frameSet, filename);
    setFilename(filename);
}

const UnTech::DocumentType& MetaSpriteDocument::documentType() const
{
    return DOCUMENT_TYPE;
}
