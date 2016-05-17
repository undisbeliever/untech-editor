#ifndef _UNTECH_MODELS_SPRITEIMPORTER_SERIALIZER_H
#define _UNTECH_MODELS_SPRITEIMPORTER_SERIALIZER_H

#include <memory>
#include <ostream>
#include <string>

/**
 * YOU SHOULD NOT CALL THIS CLASS DIRECTLY.
 *
 * It is called by the SpriteImporterDocument class.
 */

namespace UnTech {

namespace Xml {
class XmlReader;
class XmlWriter;
struct XmlTag;
}

namespace SpriteImporter {

class FrameSet;

namespace Serializer {

// NOTE: FrameSet MUST be empty
void readFile(FrameSet& frameSet, const std::string& filename);

void writeFile(const FrameSet& frameSet, const std::string& filename);
}
}
}

#endif
