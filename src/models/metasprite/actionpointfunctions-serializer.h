/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "common.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"

namespace UnTech {
namespace MetaSprite {

void readActionPointFunction(const Xml::XmlTag* tag, NamedList<ActionPointFunction>& actionPointFunctions);
void writeActionPointFunctions(Xml::XmlWriter& xml, const NamedList<ActionPointFunction>& actionPointFunctions);

}
}
