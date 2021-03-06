/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "frameset-exportorder.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"

namespace UnTech::MetaSprite {

std::unique_ptr<FrameSetExportOrder> readFrameSetExportOrder(Xml::XmlReader& xml);
void writeFrameSetExportOrder(Xml::XmlWriter& xml, const FrameSetExportOrder& eo);

}
