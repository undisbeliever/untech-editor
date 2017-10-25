/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "animation-frames-input.h"
#include "resources.h"
#include "models/common/xml/xmlreader.h"

namespace UnTech {
namespace Resources {

// raises exception on error
std::unique_ptr<ResourcesFile> loadResourcesFile(const std::string& filename);

// raises exception on error
// `afi` must be empty, xml/tag points to an <animation-frames> tag
void readAnimationFramesInput(AnimationFramesInput& afi,
                              Xml::XmlReader& xml, const Xml::XmlTag* tag);
}
}
