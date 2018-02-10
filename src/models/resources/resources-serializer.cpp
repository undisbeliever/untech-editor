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

static std::shared_ptr<PaletteInput> readPalette(const XmlTag* tag)
{
    assert(tag->name == "palette");

    auto palette = std::make_shared<PaletteInput>();

    palette->name = tag->getAttributeId("name");
    palette->paletteImageFilename = tag->getAttributeFilename("image");
    palette->rowsPerFrame = tag->getAttributeUnsigned("rows-per-frame");
    palette->skipFirstFrame = tag->getAttributeBoolean("skip-first");

    if (tag->hasAttribute("animation-delay")) {
        palette->animationDelay = tag->getAttributeUnsigned("animation-delay");
    }

    palette->paletteImage.loadPngImage(palette->paletteImageFilename);

    return palette;
}

static std::unique_ptr<ResourcesFile> readResourcesFile(XmlReader& xml, const XmlTag* tag)
{
    if (tag == nullptr || tag->name != "resources") {
        throw xml_error(xml, "Not a Resources file (expected <resources> tag)");
    }

    bool readMetaTileEngineSettingsTag = false;

    auto resources = std::make_unique<ResourcesFile>();

    resources->blockSize = tag->getAttributeUnsigned("block-size");
    resources->blockCount = tag->getAttributeUnsigned("block-count");

    while (auto childTag = xml.parseTag()) {
        if (childTag->name == "palette") {
            resources->palettes.push_back(readPalette(childTag.get()));
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

void writeResourcesFile(XmlWriter& xml, const ResourcesFile& res)
{
    xml.writeTag("resources");

    xml.writeTagAttribute("block-size", res.blockSize);
    xml.writeTagAttribute("block-count", res.blockCount);

    MetaTiles::writeEngineSettings(xml, res.metaTileEngineSettings);

    for (const std::shared_ptr<PaletteInput>& p : res.palettes) {
        assert(p != nullptr);

        xml.writeTag("palette");
        xml.writeTagAttribute("name", p->name);
        xml.writeTagAttributeFilename("image", p->paletteImageFilename);
        xml.writeTagAttribute("rows-per-frame", p->rowsPerFrame);
        xml.writeTagAttribute("skip-first", p->skipFirstFrame);

        if (p->animationDelay > 0) {
            xml.writeTagAttribute("animation-delay", p->animationDelay);
        }

        xml.writeCloseTag();
    }

    for (const std::string& mtFilename : res.metaTileTilesetFilenames) {
        xml.writeTag("metatile-tileset");
        xml.writeTagAttributeFilename("src", mtFilename);
        xml.writeCloseTag();
    }

    xml.writeCloseTag();
}

std::unique_ptr<ResourcesFile> readResourcesFile(XmlReader& xml)
{
    try {
        std::unique_ptr<XmlTag> tag = xml.parseTag();
        return readResourcesFile(xml, tag.get());
    }
    catch (const std::exception& ex) {
        throw xml_error(xml, "Error loading resources file", ex);
    }
}

std::unique_ptr<ResourcesFile> loadResourcesFile(const std::string& filename)
{
    auto xml = XmlReader::fromFile(filename);
    return readResourcesFile(*xml);
}

void saveResourcesFile(const ResourcesFile& res, const std::string& filename)
{
    AtomicOfStream file(filename);
    XmlWriter xml(file, filename, "untech");
    writeResourcesFile(xml, res);
    file.commit();
}
}
}