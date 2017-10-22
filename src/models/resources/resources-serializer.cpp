/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resources.h"
#include "models/common/atomicofstream.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"
#include <cassert>

using namespace UnTech;
using namespace UnTech::Xml;

namespace UnTech {
namespace Resources {

const std::string ResourcesFile::FILE_EXTENSION = "utres";

static void readPalette(PaletteInput& palette, const XmlTag* tag)
{
    assert(tag->name == "palette");

    palette.name = tag->getAttributeId("name");
    palette.paletteImageFilename = tag->getAttributeFilename("image");
    palette.rowsPerFrame = tag->getAttributeUnsigned("rows-per-frame");
    palette.skipFirstFrame = tag->getAttributeBoolean("skip-first");

    if (tag->hasAttribute("animation-delay")) {
        palette.animationDelay = tag->getAttributeUnsigned("animation-delay");
    }

    palette.paletteImage.loadPngImage(palette.paletteImageFilename);
}

static std::unique_ptr<ResourcesFile> readResourcesFile(XmlReader& xml, const XmlTag* tag)
{
    if (tag == nullptr || tag->name != "resources") {
        throw xml_error(xml, "Not a Resources file (expected <resources> tag)");
    }

    auto resources = std::make_unique<ResourcesFile>();

    while (auto childTag = xml.parseTag()) {
        if (childTag->name == "palette") {
            resources->palettes.emplace_back();
            readPalette(resources->palettes.back(), childTag.get());
        }
        else {
            throw unknown_tag_error(*childTag);
        }

        xml.parseCloseTag();
    }

    return resources;
}

std::unique_ptr<ResourcesFile> loadResourcesFile(const std::string& filename)
{
    auto xml = XmlReader::fromFile(filename);
    try {
        std::unique_ptr<XmlTag> tag = xml->parseTag();
        return readResourcesFile(*xml, tag.get());
    }
    catch (const std::exception& ex) {
        throw xml_error(*xml, "Error loading resources file", ex);
    }
}
}
}
