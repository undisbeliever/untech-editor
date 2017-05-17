/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

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
