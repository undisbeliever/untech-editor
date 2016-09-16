#pragma once

#include "metasprite.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"

namespace UnTech {
namespace MetaSprite {
namespace MetaSprite {

std::unique_ptr<FrameSet> readFrameSet(Xml::XmlReader& xml, const Xml::XmlTag* tag);
void writeFrameSet(Xml::XmlWriter& xml, const FrameSet& frameSet);
}
}
}
