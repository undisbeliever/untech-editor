#pragma once

#include "animation.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"
#include <string>

namespace UnTech {
namespace MetaSpriteCommon {

void readAnimation(Xml::XmlReader& xml, const Xml::XmlTag* tag,
                   Animation::list_t& animations);

void writeAnimations(Xml::XmlWriter& xml, const Animation::list_t& animations);
}
}
