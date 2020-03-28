/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
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

static const EnumMap<TileCollisionType> tileCollisonTypeEnumMap = {
    { "", TileCollisionType::EMPTY },
    { "solid", TileCollisionType::SOLID },
    { "drs", TileCollisionType::DOWN_RIGHT_SLOPE },
    { "dls", TileCollisionType::DOWN_LEFT_SLOPE },
    { "drss", TileCollisionType::DOWN_RIGHT_SHORT_SLOPE },
    { "drts", TileCollisionType::DOWN_RIGHT_TALL_SLOPE },
    { "dlts", TileCollisionType::DOWN_LEFT_TALL_SLOPE },
    { "dlss", TileCollisionType::DOWN_LEFT_SHORT_SLOPE },
    { "dp", TileCollisionType::DOWN_PLATFORM },
    { "up", TileCollisionType::UP_PLATFORM },
    { "urs", TileCollisionType::UP_RIGHT_SLOPE },
    { "uls", TileCollisionType::UP_LEFT_SLOPE },
    { "urss", TileCollisionType::UP_RIGHT_SHORT_SLOPE },
    { "urts", TileCollisionType::UP_RIGHT_TALL_SLOPE },
    { "ults", TileCollisionType::UP_LEFT_TALL_SLOPE },
    { "ulss", TileCollisionType::UP_LEFT_SHORT_SLOPE },
    { "es", TileCollisionType::END_SLOPE },

    { "empty", TileCollisionType::EMPTY },

    { "EMPTY", TileCollisionType::EMPTY },
    { "SOLID", TileCollisionType::SOLID },
    { "DOWN_RIGHT_SLOPE", TileCollisionType::DOWN_RIGHT_SLOPE },
    { "DOWN_LEFT_SLOPE", TileCollisionType::DOWN_LEFT_SLOPE },
    { "DOWN_RIGHT_SHORT_SLOPE", TileCollisionType::DOWN_RIGHT_SHORT_SLOPE },
    { "DOWN_RIGHT_TALL_SLOPE", TileCollisionType::DOWN_RIGHT_TALL_SLOPE },
    { "DOWN_LEFT_TALL_SLOPE", TileCollisionType::DOWN_LEFT_TALL_SLOPE },
    { "DOWN_LEFT_SHORT_SLOPE", TileCollisionType::DOWN_LEFT_SHORT_SLOPE },
    { "DOWN_PLATFORM", TileCollisionType::DOWN_PLATFORM },
    { "UP_PLATFORM", TileCollisionType::UP_PLATFORM },
    { "UP_RIGHT_SLOPE", TileCollisionType::UP_RIGHT_SLOPE },
    { "UP_LEFT_SLOPE", TileCollisionType::UP_LEFT_SLOPE },
    { "UP_RIGHT_SHORT_SLOPE", TileCollisionType::UP_RIGHT_SHORT_SLOPE },
    { "UP_RIGHT_TALL_SLOPE", TileCollisionType::UP_RIGHT_TALL_SLOPE },
    { "UP_LEFT_TALL_SLOPE", TileCollisionType::UP_LEFT_TALL_SLOPE },
    { "UP_LEFT_SHORT_SLOPE", TileCollisionType::UP_LEFT_SHORT_SLOPE },
    { "END_SLOPE", TileCollisionType::END_SLOPE },
};

const std::string MetaTileTilesetInput::FILE_EXTENSION = "utmt";

void readEngineSettings(EngineSettings& settings, const XmlTag* tag)
{
    settings.maxMapSize = tag->getAttributeUnsigned("max-map-size");
}

static grid<uint8_t> readMetaTileGrid(XmlReader& xml, const XmlTag* tag)
{
    const unsigned width = tag->getAttributeUnsigned("width", 1, MAX_GRID_WIDTH);
    const unsigned height = tag->getAttributeUnsigned("height", 1, MAX_GRID_HEIGHT);
    const unsigned expectedDataSize = width * height;

    auto data = xml.parseBase64();

    if (data.size() != expectedDataSize) {
        throw xml_error(*tag, stringBuilder("Invalid data size. Got ", data.size(), " bytes, expected ", expectedDataSize, "."));
    }

    return grid<uint8_t>(width, height, std::move(data));
}

static void writeMetaTileGrid(XmlWriter& xml, const std::string& tagName, const grid<uint8_t>& mtGrid)
{
    if (mtGrid.empty()) {
        return;
    }

    xml.writeTag(tagName);
    xml.writeTagAttribute("width", mtGrid.width());
    xml.writeTagAttribute("height", mtGrid.height());
    xml.writeBase64(mtGrid.gridData());
    xml.writeCloseTag();
}

static void readTileProperties(const XmlTag* tag, MetaTileTilesetInput& tilesetInput)
{
    // <tile> tag

    unsigned i = tag->getAttributeUnsigned("t", 0, N_METATILES - 1);
    tilesetInput.tileCollisions.at(i) = tag->getAttributeOptionalEnum("collision", tileCollisonTypeEnumMap, TileCollisionType::EMPTY);
}

static void writeTileProperties(XmlWriter& xml, const MetaTileTilesetInput& tilesetInput)
{
    for (unsigned i = 0; i < N_METATILES; i++) {
        const auto& tc = tilesetInput.tileCollisions.at(i);

        if (tc != TileCollisionType::EMPTY) {
            xml.writeTag("tile");
            xml.writeTagAttribute("t", i);
            xml.writeTagAttributeEnum("collision", tc, tileCollisonTypeEnumMap);
            xml.writeCloseTag();
        }
    }
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
        else if (childTag->name == "tile") {
            readTileProperties(childTag.get(), *tilesetInput);
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

    writeTileProperties(xml, input);

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

std::unique_ptr<MetaTileTilesetInput> loadMetaTileTilesetInput(const std::filesystem::path& filename)
{
    auto xml = XmlReader::fromFile(filename);
    return readMetaTileTilesetInput(*xml);
}

std::unique_ptr<MetaTileTilesetInput> loadMetaTileTilesetInput(const std::filesystem::path& filename,
                                                               ErrorList& err)
{
    try {
        auto xml = XmlReader::fromFile(filename);
        return readMetaTileTilesetInput(*xml);
    }
    catch (const std::exception& ex) {
        err.addErrorString(ex.what());
        return nullptr;
    }
}

void saveMetaTileTilesetInput(const MetaTileTilesetInput& input, const std::filesystem::path& filename)
{
    AtomicOfStream file(filename);
    XmlWriter xml(file, filename, "untech");
    writeMetaTileTilesetInput(xml, input);
    file.commit();
}
}
}
