#ifndef _UNTECH_MODELS_SPRITEIMPORTER_SERIALIZER_H
#define _UNTECH_MODELS_SPRITEIMPORTER_SERIALIZER_H

#include "frameset.h"
#include <string>
#include <ostream>

namespace UnTech {

namespace Xml {
class XmlReader;
class XmlWriter;
struct XmlTag;
}

namespace SpriteImporter {

class FrameSet;

namespace Serializer {

void readFile(FrameSet::list_t& frameSetContainer, const std::string& filename);
void readSpriteImporter(FrameSet::list_t& frameSetContainer, Xml::XmlReader& xml, const Xml::XmlTag* currentTag);

void writeFile(const FrameSet::list_t& frameSetContainer, std::ostream& file);
void writeFile(const FrameSet::list_t& frameSetContainer, const std::string& filename);
void writeSpriteImporter(const FrameSet::list_t& frameSetContainer, Xml::XmlWriter& xml);
}
}
}

#endif
