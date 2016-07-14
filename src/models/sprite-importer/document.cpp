#include "document.h"

#include "serializer.h"
#include "models/sprite-importer.h"

using namespace UnTech::SpriteImporter;

const UnTech::DocumentType SpriteImporterDocument::DOCUMENT_TYPE = {
    "UnTech Sprite Importer Document",
    "utsi"
};

SpriteImporterDocument::SpriteImporterDocument()
    : Document()
    , _frameSet(*this)
{
}

SpriteImporterDocument::SpriteImporterDocument(const std::string& filename)
    : Document(filename)
    , _frameSet(*this)
{
    Serializer::readFile(_frameSet, filename);
}

void SpriteImporterDocument::writeDataFile(const std::string& filename)
{
    Serializer::writeFile(_frameSet, filename);
    setFilename(filename);
}

const UnTech::DocumentType& SpriteImporterDocument::documentType() const
{
    return DOCUMENT_TYPE;
}
