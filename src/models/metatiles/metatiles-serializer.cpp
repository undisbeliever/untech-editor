/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
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

static void readInteractiveTileFunction(const XmlTag& tag, NamedList<InteractiveTileFunctionTable>& tileFunctions)
{
    tileFunctions.insert_back();
    InteractiveTileFunctionTable& ft = tileFunctions.back();

    ft.name = tag.getAttributeOptionalId("name");

    if (tag.hasAttribute("tint")) {
        ft.tint = UnTech::rgba::fromRgbHex(tag.getAttributeUnsignedHex("tint"));
    }
    if (tag.hasAttribute("color")) {
        ft.tint = UnTech::rgba::fromRgbHex(tag.getAttributeUnsignedHex("color"));
    }
}

void readInteractiveTiles(XmlReader& xml, const XmlTag* tag, InteractiveTiles& interactiveTiles)
{
    assert(tag->name == "interactive-tiles");

    while (auto childTag = xml.parseTag()) {
        if (childTag->name == "tile-function-table") {
            readInteractiveTileFunction(*childTag, interactiveTiles.functionTables);
        }
        else {
            throw unknown_tag_error(*childTag);
        }

        xml.parseCloseTag();
    }
}

void writeInteractiveTiles(XmlWriter& xml, const InteractiveTiles& interactiveTiles)
{
    xml.writeTag("interactive-tiles");
    for (auto& ft : interactiveTiles.functionTables) {
        xml.writeTag("tile-function-table");
        xml.writeTagAttribute("name", ft.name);
        xml.writeTagAttributeHex("tint", ft.tint.rgbHex(), 6);
        xml.writeCloseTag();
    }
    xml.writeCloseTag();
}

grid<uint8_t> readMetaTileGrid(XmlReader& xml, const XmlTag* tag)
{
    const unsigned width = tag->getAttributeUnsigned("width", 1, MAX_GRID_WIDTH);
    const unsigned height = tag->getAttributeUnsigned("height", 1, MAX_GRID_HEIGHT);
    const unsigned expectedDataSize = width * height;

    return grid<uint8_t>(width, height,
                         xml.parseBase64OfKnownSize(expectedDataSize));
}

void writeMetaTileGrid(XmlWriter& xml, const std::string& tagName, const grid<uint8_t>& mtGrid)
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
    tilesetInput.tileFunctionTables.at(i) = tag->getAttributeOptionalId("ft");
}

static void writeTileProperties(XmlWriter& xml, const MetaTileTilesetInput& tilesetInput)
{
    for (unsigned i = 0; i < N_METATILES; i++) {
        const auto& tc = tilesetInput.tileCollisions.at(i);
        const auto& ft = tilesetInput.tileFunctionTables.at(i);

        if (tc != TileCollisionType::EMPTY || ft.isValid()) {
            xml.writeTag("tile");
            xml.writeTagAttribute("t", i);
            if (tc != TileCollisionType::EMPTY) {
                xml.writeTagAttributeEnum("collision", tc, tileCollisonTypeEnumMap);
            }
            xml.writeTagAttributeOptional("ft", ft);
            xml.writeCloseTag();
        }
    }
}

static void readTilePriorities(XmlReader& xml, const XmlTag*, MetaTileTilesetInput& tilesetInput)
{
    // <tile-priorities> tag
    xml.parseBase64ToByteArray(tilesetInput.tilePriorities.data);
}

static void writeTilePriorities(XmlWriter& xml, const TilePriorities& tilePriorities)
{
    xml.writeTag("tile-priorities");
    xml.writeBase64(tilePriorities.data);
    xml.writeCloseTag();
}

static void readCrumblingTilesChain(const XmlTag* tag, MetaTileTilesetInput& tilesetInput)
{
    assert(tag->name == "crumbling-tile");

    const std::string chainName = tag->getAttribute("chain");
    if (chainName.size() != 1 || chainName.front() < 'a' || chainName.front() >= 'a' + int(N_CRUMBLING_TILE_CHAINS)) {
        throw xml_error(*tag, "Unknown crumbling tiles chain id");
    }
    const unsigned chainId = chainName.front() - 'a';

    CrumblingTileChain& chain = tilesetInput.crumblingTiles.at(chainId);

    chain.firstTileId = tag->getAttributeUint8("first-tile");
    chain.firstDelay = tag->getAttributeUint16("first-delay");

    chain.secondTileId = tag->getAttributeUint8("second-tile");

    if (tag->getAttributeBoolean("no-third-tile") == false) {
        chain.secondDelay = tag->getAttributeUint16("second-delay");
        chain.thirdTileId = tag->getAttributeUint8("third-tile");
    }
    else {
        chain.secondDelay = 0xffff;
        chain.thirdTileId = 0;
    }
}

static void writeCrumblingTilesChains(XmlWriter& xml, const std::array<CrumblingTileChain, N_CRUMBLING_TILE_CHAINS>& chains)
{
    static_assert(N_CRUMBLING_TILE_CHAINS < 26);
    std::string chainName{ "a" };

    for (auto& chain : chains) {
        xml.writeTag("crumbling-tile");

        xml.writeTagAttribute("chain", chainName);

        xml.writeTagAttribute("first-tile", chain.firstTileId);
        xml.writeTagAttribute("first-delay", chain.firstDelay);

        xml.writeTagAttribute("second-tile", chain.secondTileId);

        if (chain.hasThirdTransition()) {
            xml.writeTagAttribute("second-delay", chain.secondDelay);
            xml.writeTagAttribute("third-tile", chain.thirdTileId);
        }
        else {
            xml.writeTagAttribute("no-third-tile", true);
        }

        xml.writeCloseTag();

        chainName.front() += 1;
    }
}

static std::unique_ptr<MetaTileTilesetInput> readMetaTileTilesetInput(XmlReader& xml, const XmlTag* tag)
{
    if (tag == nullptr || tag->name != "metatile-tileset") {
        throw xml_error(xml, "Not a Resources file (expected <metatile-tileset> tag)");
    }

    bool readAnimationFramesTag = false;
    bool readScratchpadTag = false;
    bool readTilePrioritiesTag = false;

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
            if (readScratchpadTag) {
                throw xml_error(*childTag, "Only one <scratchpad> tag allowed");
            }
            readScratchpadTag = true;

            tilesetInput->scratchpad = readMetaTileGrid(xml, childTag.get());
        }
        else if (childTag->name == "tile-priorities") {
            if (readTilePrioritiesTag) {
                throw xml_error(*childTag, "Only one <tile-priorities> tag allowed");
            }
            readTilePrioritiesTag = true;

            readTilePriorities(xml, childTag.get(), *tilesetInput);
        }
        else if (childTag->name == "crumbling-tile") {
            readCrumblingTilesChain(childTag.get(), *tilesetInput);
        }
        else {
            throw unknown_tag_error(*childTag);
        }

        xml.parseCloseTag();
    }

    return tilesetInput;
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

    writeCrumblingTilesChains(xml, input.crumblingTiles);
    writeTileProperties(xml, input);
    writeTilePriorities(xml, input.tilePriorities);

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
