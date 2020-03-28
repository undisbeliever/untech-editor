/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "entityromdata.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"

namespace UnTech {
namespace Entity {

void readEntityRomData(Xml::XmlReader& xml, const Xml::XmlTag* tag, EntityRomData& entityRomData);
void writeEntityRomData(Xml::XmlWriter& xml, const EntityRomData& romData);

}
}
