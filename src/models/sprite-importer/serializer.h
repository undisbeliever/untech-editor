#ifndef _UNTECH_MODELS_SPRITEIMPORTER_SERIALIZER_H
#define _UNTECH_MODELS_SPRITEIMPORTER_SERIALIZER_H

#include "../common/namedlist.h"
#include <string>

namespace UnTech {

namespace Xml {
class XmlReader;
class XmlTag;
}

namespace SpriteImporter {

class FrameSet;

namespace Serializer {

void readFile(NamedList<FrameSet>& frameSetContainer, const std::string& filename);
void readSpriteImporter(NamedList<FrameSet>& frameSetContainer, Xml::XmlReader& xml, const Xml::XmlTag* currentTag);
}
}
}

#endif
