/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "animation-frames-input.h"
#include "models/common/namedlist.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"

namespace UnTech {
namespace Resources {

// raises exception on error
void readPalette(const Xml::XmlTag* tag, NamedList<PaletteInput>& palettes);

// raises exception on error
void writePalettes(Xml::XmlWriter& xml, const NamedList<PaletteInput>& palettes);

// raises exception on error
// `afi` must be empty, xml/tag points to an <animation-frames> tag
void readAnimationFramesInput(AnimationFramesInput& afi,
                              Xml::XmlReader& xml, const Xml::XmlTag* tag);

// raises exception on error
void writeAnimationFramesInput(Xml::XmlWriter& xml, const AnimationFramesInput& afi);
}
}
