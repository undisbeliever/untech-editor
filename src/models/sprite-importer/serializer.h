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

void readSpriteImporter(NamedList<FrameSet>& framesetContainer, Xml::XmlReader& xml, const Xml::XmlTag* tag, const std::string& fileDir, const std::string& fileName);
}
}
}

#endif
