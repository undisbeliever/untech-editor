#pragma once

#include "animation.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"

namespace UnTech {
namespace MetaSprite {
namespace Animation {

void readAnimation(Xml::XmlReader& xml, const Xml::XmlTag* tag,
                   Animation::map_t& animations);

void writeAnimations(Xml::XmlWriter& xml,
                     const Animation::map_t& animations);
}
}
}