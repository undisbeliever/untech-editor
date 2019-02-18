/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resources-serializer.h"
#include "models/common/atomicofstream.h"
#include "models/common/xml/xmlreader.h"
#include "models/metatiles/metatiles-serializer.h"
#include <cassert>

using namespace UnTech;
using namespace UnTech::Xml;

namespace UnTech {
namespace Resources {

void readPalette(const XmlTag* tag, NamedList<PaletteInput>& palettes)
{
    assert(tag->name == "palette");

    palettes.insert_back();
    auto& palette = palettes.back();

    palette.name = tag->getAttributeId("name");
    palette.paletteImageFilename = tag->getAttributeFilename("image");
    palette.rowsPerFrame = tag->getAttributeUnsigned("rows-per-frame");
    palette.skipFirstFrame = tag->getAttributeBoolean("skip-first");

    if (tag->hasAttribute("animation-delay")) {
        palette.animationDelay = tag->getAttributeUnsigned("animation-delay");
    }
}

void writePalettes(XmlWriter& xml, const NamedList<PaletteInput>& palettes)
{
    for (const auto& p : palettes) {
        xml.writeTag("palette");
        xml.writeTagAttribute("name", p.name);
        xml.writeTagAttributeFilename("image", p.paletteImageFilename);
        xml.writeTagAttribute("rows-per-frame", p.rowsPerFrame);
        xml.writeTagAttribute("skip-first", p.skipFirstFrame);

        if (p.animationDelay > 0) {
            xml.writeTagAttribute("animation-delay", p.animationDelay);
        }

        xml.writeCloseTag();
    }
}

void readAnimationFramesInput(AnimationFramesInput& afi, XmlReader& xml, const XmlTag* tag)
{
    assert(tag->name == "animation-frames");
    assert(afi.frameImageFilenames.empty());

    afi.bitDepth = tag->getAttributeUnsigned("bit-depth", 2, 8);
    afi.animationDelay = tag->getAttributeUnsigned("animation-delay");
    afi.addTransparentTile = tag->getAttributeBoolean("add-transparent-tile");

    while (auto childTag = xml.parseTag()) {
        if (childTag->name == "frame") {
            const std::string filename = childTag->getAttributeFilename("image");
            afi.frameImageFilenames.emplace_back(filename);
        }
        else {
            throw xml_error(xml, "Expected <frame> tag");
        }

        xml.parseCloseTag();
    }
}

void writeAnimationFramesInput(XmlWriter& xml, const AnimationFramesInput& afi)
{
    xml.writeTag("animation-frames");

    xml.writeTagAttribute("bit-depth", afi.bitDepth);
    xml.writeTagAttribute("animation-delay", afi.animationDelay);
    xml.writeTagAttribute("add-transparent-tile", afi.addTransparentTile);

    for (const std::string& fiFilename : afi.frameImageFilenames) {
        xml.writeTag("frame");
        xml.writeTagAttributeFilename("image", fiFilename);
        xml.writeCloseTag();
    }

    xml.writeCloseTag();
}

}
}
