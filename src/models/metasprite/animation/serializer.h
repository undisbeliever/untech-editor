/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "animation.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"

namespace UnTech::MetaSprite::Animation {

void readAnimation(Xml::XmlReader& xml, const Xml::XmlTag& tag,
                   NamedList<Animation>& animations);

void writeAnimations(Xml::XmlWriter& xml,
                     const NamedList<Animation>& animations);

}
