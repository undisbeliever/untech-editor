#pragma once

#include "spriteimporter.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"

namespace UnTech {
namespace MetaSprite {
namespace SpriteImporter {

void readFrameSet(Xml::XmlReader& xml, const Xml::XmlTag* tag,
                  FrameSet& frameSet);

void writeFrameSet(Xml::XmlWriter& xml,
                   const FrameSet& frameSet);
}
}
}
