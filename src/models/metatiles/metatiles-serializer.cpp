/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "metatiles-serializer.h"
#include "models/common/xml/xmlreader.h"
#include "models/resources/resources-serializer.h"
#include <cassert>

using namespace UnTech;
using namespace UnTech::Xml;

namespace UnTech {
namespace MetaTiles {

const std::string MetaTileTilesetInput::FILE_EXTENSION = "utmt";

void readEngineSettings(EngineSettings& settings, const XmlTag* tag)
{
    assert("metatile-engine-settings");

    settings.maxMapSize = tag->getAttributeUnsigned("max-map-size");
    settings.nMetaTiles = tag->getAttributeUnsigned("n-metatiles");
}

static std::unique_ptr<MetaTileTilesetInput> readMetaTileTilesetInput(XmlReader& xml, const XmlTag* tag)
{
    if (tag == nullptr || tag->name != "metatile-tileset") {
        throw xml_error(xml, "Not a Resources file (expected <metatile-tileset> tag)");
    }

    bool readAnimationFramesTag = false;

    auto tilesetInput = std::make_unique<MetaTileTilesetInput>();

    tilesetInput->name = tag->getAttributeId("name");

    while (auto childTag = xml.parseTag()) {
        if (childTag->name == "palette") {
            tilesetInput->palettes.emplace_back(childTag->getAttributeId("name"));
        }
        else if (childTag->name == "animation-frames") {
            if (readAnimationFramesTag) {
                throw xml_error(*childTag, "Only one <animation-frames> tag allowed");
            }
            readAnimationFramesTag = true;

            Resources::readAnimationFramesInput(tilesetInput->animationFrames, xml, childTag.get());
        }
        else {
            throw unknown_tag_error(*childTag);
        }

        xml.parseCloseTag();
    }

    return tilesetInput;
}

std::unique_ptr<MetaTileTilesetInput> loadMetaTileTilesetInput(const std::string& filename)
{
    auto xml = XmlReader::fromFile(filename);
    try {
        std::unique_ptr<XmlTag> tag = xml->parseTag();
        return readMetaTileTilesetInput(*xml, tag.get());
    }
    catch (const std::exception& ex) {
        throw xml_error(*xml, "Error loading metatile tileset file", ex);
    }
}
}
}
