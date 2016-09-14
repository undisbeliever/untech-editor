#pragma once

#include "metasprite.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"

namespace UnTech {
namespace MetaSprite {
namespace MetaSprite {

// NOTE: FrameSet MUST be empty
void readFrameSet(Xml::XmlReader& xml, const Xml::XmlTag* tag,
                  FrameSet& frameSet);

void writeFrameSet(Xml::XmlWriter& xml,
                   const FrameSet& frameSet);
}
}
}
