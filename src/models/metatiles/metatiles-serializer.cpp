/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "metatiles-serializer.h"
#include "models/common/atomicofstream.h"
#include "models/common/bytevectorhelper.h"
#include "models/common/externalfilelist.h"
#include "models/common/xml/xmlreader.h"
#include "models/resources/resources-serializer.h"
#include <cassert>

using namespace UnTech;
using namespace UnTech::Xml;

namespace UnTech {

template <>
void UnTech::ExternalFileItem<MetaTiles::MetaTileTilesetInput>::loadFile()
{
    value = MetaTiles::loadMetaTileTilesetInput(filename);
}

namespace MetaTiles {

const std::string MetaTileTilesetInput::FILE_EXTENSION = "utmt";

void readEngineSettings(EngineSettings& settings, const XmlTag* tag)
{
    assert("metatile-engine-settings");

    settings.maxMapSize = tag->getAttributeUnsigned("max-map-size");
    settings.nMetaTiles = tag->getAttributeUnsigned("n-metatiles");
}

static grid<uint16_t> readMetaTileGrid(XmlReader& xml, const XmlTag* tag)
{
    grid<uint16_t> mtGrid(tag->getAttributeUnsigned("width", 1, MAX_GRID_WIDTH),
                          tag->getAttributeUnsigned("height", 1, MAX_GRID_HEIGHT));

    const auto data = xml.parseBase64();

    if (data.size() != mtGrid.cellCount() * 2) {
        const std::string msg = "Invalid data size. Got " + std::to_string(data.size())
                                + " bytes, expected " + std::to_string(mtGrid.cellCount() * 2) + ".";
        throw xml_error(*tag, msg.c_str());
    }

    auto dataIt = data.begin();
    for (auto& t : mtGrid) {
        uint8_t b1 = *dataIt++;
        uint8_t b2 = *dataIt++;

        t = b1 | (b2 << 8);
    }
    assert(dataIt == data.end());

    return mtGrid;
}

static void writeMetaTileGrid(XmlWriter& xml, const std::string& tagName, const grid<uint16_t>& mtGrid)
{
    if (mtGrid.empty()) {
        return;
    }

    std::vector<uint8_t> data;
    data.reserve(mtGrid.cellCount() * 2);

    for (const auto t : mtGrid) {
        writeUint16(data, t);
    }
    assert(data.size() == mtGrid.cellCount() * 2);

    xml.writeTag(tagName);
    xml.writeTagAttribute("width", mtGrid.width());
    xml.writeTagAttribute("height", mtGrid.height());
    xml.writeBase64(data);
    xml.writeCloseTag();
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
        else if (childTag->name == "scratchpad") {
            tilesetInput->scratchpad = readMetaTileGrid(xml, childTag.get());
        }
        else {
            throw unknown_tag_error(*childTag);
        }

        xml.parseCloseTag();
    }

    return tilesetInput;
}

void writeEngineSettings(XmlWriter& xml, const EngineSettings& settings)
{
    xml.writeTag("metatile-engine-settings");
    xml.writeTagAttribute("max-map-size", settings.maxMapSize);
    xml.writeTagAttribute("n-metatiles", settings.nMetaTiles);
    xml.writeCloseTag();
}

void writeMetaTileTilesetInput(XmlWriter& xml, const MetaTileTilesetInput& input)
{
    xml.writeTag("metatile-tileset");

    xml.writeTagAttribute("name", input.name);

    for (const idstring& pName : input.palettes) {
        xml.writeTag("palette");
        xml.writeTagAttribute("name", pName);
        xml.writeCloseTag();
    }

    Resources::writeAnimationFramesInput(xml, input.animationFrames);

    writeMetaTileGrid(xml, "scratchpad", input.scratchpad);

    xml.writeCloseTag();
}

std::unique_ptr<MetaTileTilesetInput> readMetaTileTilesetInput(XmlReader& xml)
{
    try {
        std::unique_ptr<XmlTag> tag = xml.parseTag();
        return readMetaTileTilesetInput(xml, tag.get());
    }
    catch (const std::exception& ex) {
        throw xml_error(xml, "Error loading metatile tileset", ex);
    }
}

std::unique_ptr<MetaTileTilesetInput> loadMetaTileTilesetInput(const std::string& filename)
{
    auto xml = XmlReader::fromFile(filename);
    return readMetaTileTilesetInput(*xml);
}

std::unique_ptr<MetaTileTilesetInput> loadMetaTileTilesetInput(const std::string& filename,
                                                               ErrorList& err)
{
    try {
        auto xml = XmlReader::fromFile(filename);
        return readMetaTileTilesetInput(*xml);
    }
    catch (const std::exception& ex) {
        err.addError(ex.what());
        return nullptr;
    }
}

void saveMetaTileTilesetInput(const MetaTileTilesetInput& input, const std::string& filename)
{
    AtomicOfStream file(filename);
    XmlWriter xml(file, filename, "untech");
    writeMetaTileTilesetInput(xml, input);
    file.commit();
}
}
}
