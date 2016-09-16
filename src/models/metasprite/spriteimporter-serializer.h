#pragma once

#include "spriteimporter.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"

namespace UnTech {
namespace MetaSprite {
namespace SpriteImporter {

std::unique_ptr<FrameSet> readFrameSet(Xml::XmlReader& xml, const Xml::XmlTag* tag);
void writeFrameSet(Xml::XmlWriter& xml, const FrameSet& frameSet);
}
}
}
