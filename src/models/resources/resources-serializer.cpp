/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resources-serializer.h"
#include "models/common/xml/xmlreader.h"
#include "models/metatiles/metatiles-serializer.h"
#include <cassert>

using namespace UnTech;
using namespace UnTech::Xml;

namespace UnTech {
namespace Resources {

const std::string ResourcesFile::FILE_EXTENSION = "utres";

void readAnimationFramesInput(AnimationFramesInput& afi, XmlReader& xml, const XmlTag* tag)
{
    assert(tag->name == "animation-frames");
    assert(afi.frameImages.empty());
    assert(afi.frameImageFilenames.empty());

    afi.bitDepth = tag->getAttributeUnsigned("bit-depth", 2, 8);
    afi.animationDelay = tag->getAttributeUnsigned("animation-delay");
    afi.addTransparentTile = tag->getAttributeBoolean("add-transparent-tile");

    while (auto childTag = xml.parseTag()) {
        if (childTag->name == "frame") {
            const std::string filename = childTag->getAttributeFilename("image");

            afi.frameImages.emplace_back();
            afi.frameImages.back().loadPngImage(filename);

            afi.frameImageFilenames.emplace_back(filename);
        }
        else {
            throw xml_error(xml, "Expected <frame> tag");
        }

        xml.parseCloseTag();
    }
}

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

    bool readMetaTileEngineSettingsTag = false;

    auto resources = std::make_unique<ResourcesFile>();

    while (auto childTag = xml.parseTag()) {
        if (childTag->name == "palette") {
            resources->palettes.emplace_back();
            readPalette(resources->palettes.back(), childTag.get());
        }
        else if (childTag->name == "metatile-tileset") {
            resources->metaTileTilesetFilenames.emplace_back(
                childTag->getAttributeFilename("src"));
        }
        else if (childTag->name == "metatile-engine-settings") {
            if (readMetaTileEngineSettingsTag) {
                throw xml_error(*childTag, "Only one <metatile-engine-settings> tag is allowed");
            }
            readMetaTileEngineSettingsTag = true;

            MetaTiles::readEngineSettings(resources->metaTileEngineSettings, childTag.get());
        }
        else {
            throw unknown_tag_error(*childTag);
        }

        xml.parseCloseTag();
    }

    if (readMetaTileEngineSettingsTag == false) {
        throw xml_error(xml, "Expected a <metatile-engine-settings> tag");
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
