/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "spriteimporter.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"

namespace UnTech {
namespace MetaSprite {
namespace SpriteImporter {

std::unique_ptr<FrameSet> readFrameSet(Xml::XmlReader& xml);
void writeFrameSet(Xml::XmlWriter& xml, const FrameSet& frameSet);
}
}
}
