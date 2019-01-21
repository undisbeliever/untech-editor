/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "framesetfile.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"

namespace UnTech {
namespace MetaSprite {

void readFrameSetFile(const Xml::XmlTag* tag, std::vector<FrameSetFile>& frameSets);
void writeFrameSetFiles(Xml::XmlWriter& xml, const std::vector<FrameSetFile>& frameSets);

}
}
