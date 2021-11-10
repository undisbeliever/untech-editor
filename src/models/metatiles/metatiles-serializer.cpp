/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "metatiles-serializer.h"
#include "models/common/bytevectorhelper.h"
#include "models/common/externalfilelist.h"
#include "models/common/u8strings.h"
#include "models/common/xml/xmlreader.h"
#include "models/resources/resources-serializer.h"
#include <cassert>

using namespace UnTech::Xml;

namespace UnTech {

template <>
void ExternalFileItem<MetaTiles::MetaTileTilesetInput>::loadFile()
{
    value = MetaTiles::loadMetaTileTilesetInput(filename);
}

}

namespace UnTech::MetaTiles {

static const EnumMap<TileCollisionType> tileCollisonTypeEnumMap = {
    { u8"", TileCollisionType::EMPTY },
    { u8"solid", TileCollisionType::SOLID },
    { u8"drs", TileCollisionType::DOWN_RIGHT_SLOPE },
    { u8"dls", TileCollisionType::DOWN_LEFT_SLOPE },
    { u8"drss", TileCollisionType::DOWN_RIGHT_SHORT_SLOPE },
    { u8"drts", TileCollisionType::DOWN_RIGHT_TALL_SLOPE },
    { u8"dlts", TileCollisionType::DOWN_LEFT_TALL_SLOPE },
    { u8"dlss", TileCollisionType::DOWN_LEFT_SHORT_SLOPE },
    { u8"dp", TileCollisionType::DOWN_PLATFORM },
    { u8"up", TileCollisionType::UP_PLATFORM },
    { u8"urs", TileCollisionType::UP_RIGHT_SLOPE },
    { u8"uls", TileCollisionType::UP_LEFT_SLOPE },
    { u8"urss", TileCollisionType::UP_RIGHT_SHORT_SLOPE },
    { u8"urts", TileCollisionType::UP_RIGHT_TALL_SLOPE },
    { u8"ults", TileCollisionType::UP_LEFT_TALL_SLOPE },
    { u8"ulss", TileCollisionType::UP_LEFT_SHORT_SLOPE },
    { u8"es", TileCollisionType::END_SLOPE },

    { u8"empty", TileCollisionType::EMPTY },

    { u8"EMPTY", TileCollisionType::EMPTY },
    { u8"SOLID", TileCollisionType::SOLID },
    { u8"DOWN_RIGHT_SLOPE", TileCollisionType::DOWN_RIGHT_SLOPE },
    { u8"DOWN_LEFT_SLOPE", TileCollisionType::DOWN_LEFT_SLOPE },
    { u8"DOWN_RIGHT_SHORT_SLOPE", TileCollisionType::DOWN_RIGHT_SHORT_SLOPE },
    { u8"DOWN_RIGHT_TALL_SLOPE", TileCollisionType::DOWN_RIGHT_TALL_SLOPE },
    { u8"DOWN_LEFT_TALL_SLOPE", TileCollisionType::DOWN_LEFT_TALL_SLOPE },
    { u8"DOWN_LEFT_SHORT_SLOPE", TileCollisionType::DOWN_LEFT_SHORT_SLOPE },
    { u8"DOWN_PLATFORM", TileCollisionType::DOWN_PLATFORM },
    { u8"UP_PLATFORM", TileCollisionType::UP_PLATFORM },
    { u8"UP_RIGHT_SLOPE", TileCollisionType::UP_RIGHT_SLOPE },
    { u8"UP_LEFT_SLOPE", TileCollisionType::UP_LEFT_SLOPE },
    { u8"UP_RIGHT_SHORT_SLOPE", TileCollisionType::UP_RIGHT_SHORT_SLOPE },
    { u8"UP_RIGHT_TALL_SLOPE", TileCollisionType::UP_RIGHT_TALL_SLOPE },
    { u8"UP_LEFT_TALL_SLOPE", TileCollisionType::UP_LEFT_TALL_SLOPE },
    { u8"UP_LEFT_SHORT_SLOPE", TileCollisionType::UP_LEFT_SHORT_SLOPE },
    { u8"END_SLOPE", TileCollisionType::END_SLOPE },
};

const std::u8string MetaTileTilesetInput::FILE_EXTENSION = u8"utmt";

static void readInteractiveTileFunction(const XmlTag& tag, NamedList<InteractiveTileFunctionTable>& tileFunctions)
{
    tileFunctions.insert_back();
    InteractiveTileFunctionTable& ft = tileFunctions.back();

    ft.name = tag.getAttributeOptionalId(u8"name");

    if (tag.hasAttribute(u8"tint")) {
        ft.tint = UnTech::rgba::fromRgbHex(tag.getAttributeUnsignedHex(u8"tint"));
    }
    if (tag.hasAttribute(u8"color")) {
        ft.tint = UnTech::rgba::fromRgbHex(tag.getAttributeUnsignedHex(u8"color"));
    }
}

void readInteractiveTiles(XmlReader& xml, const XmlTag& tag, InteractiveTiles& interactiveTiles)
{
    assert(tag.name == u8"interactive-tiles");

    while (const auto childTag = xml.parseTag()) {
        if (childTag.name == u8"tile-function-table") {
            readInteractiveTileFunction(childTag, interactiveTiles.functionTables);
        }
        else {
            throw unknown_tag_error(childTag);
        }

        xml.parseCloseTag();
    }
}

void writeInteractiveTiles(XmlWriter& xml, const InteractiveTiles& interactiveTiles)
{
    xml.writeTag(u8"interactive-tiles");
    for (auto& ft : interactiveTiles.functionTables) {
        xml.writeTag(u8"tile-function-table");
        xml.writeTagAttribute(u8"name", ft.name);
        xml.writeTagAttributeHex6(u8"tint", ft.tint.rgbHex());

        xml.writeCloseTag();
    }
    xml.writeCloseTag();
}

grid<uint8_t> readMetaTileGrid(XmlReader& xml, const XmlTag& tag)
{
    const unsigned width = tag.getAttributeUnsigned(u8"width", 1, MAX_GRID_WIDTH);
    const unsigned height = tag.getAttributeUnsigned(u8"height", 1, MAX_GRID_HEIGHT);
    const unsigned expectedDataSize = width * height;

    return grid<uint8_t>(width, height,
                         xml.parseBase64OfKnownSize(expectedDataSize));
}

void writeMetaTileGrid(XmlWriter& xml, const std::u8string& tagName, const grid<uint8_t>& mtGrid)
{
    if (mtGrid.empty()) {
        return;
    }

    xml.writeTag(tagName);
    xml.writeTagAttribute(u8"width", mtGrid.width());
    xml.writeTagAttribute(u8"height", mtGrid.height());
    xml.writeBase64(mtGrid.gridData());
    xml.writeCloseTag();
}

static void readTileProperties(const XmlTag& tag, MetaTileTilesetInput& tilesetInput)
{
    // <tile> tag

    unsigned i = tag.getAttributeUnsigned(u8"t", 0, N_METATILES - 1);
    tilesetInput.tileCollisions.at(i) = tag.getAttributeOptionalEnum(u8"collision", tileCollisonTypeEnumMap, TileCollisionType::EMPTY);
    tilesetInput.tileFunctionTables.at(i) = tag.getAttributeOptionalId(u8"ft");
}

static void writeTileProperties(XmlWriter& xml, const MetaTileTilesetInput& tilesetInput)
{
    for (const auto i : range(N_METATILES)) {
        const auto& tc = tilesetInput.tileCollisions.at(i);
        const auto& ft = tilesetInput.tileFunctionTables.at(i);

        if (tc != TileCollisionType::EMPTY || ft.isValid()) {
            xml.writeTag(u8"tile");
            xml.writeTagAttribute(u8"t", unsigned(i));
            if (tc != TileCollisionType::EMPTY) {
                xml.writeTagAttributeEnum(u8"collision", tc, tileCollisonTypeEnumMap);
            }
            xml.writeTagAttributeOptional(u8"ft", ft);
            xml.writeCloseTag();
        }
    }
}

static void readTilePriorities(XmlReader& xml, const XmlTag&, MetaTileTilesetInput& tilesetInput)
{
    // <tile-priorities> tag
    xml.parseBase64ToByteArray(tilesetInput.tilePriorities.data);
}

static void writeTilePriorities(XmlWriter& xml, const TilePriorities& tilePriorities)
{
    xml.writeTag(u8"tile-priorities");
    xml.writeBase64(tilePriorities.data);
    xml.writeCloseTag();
}

static void readCrumblingTilesChain(const XmlTag& tag, MetaTileTilesetInput& tilesetInput)
{
    assert(tag.name == u8"crumbling-tile");

    const std::u8string chainName = tag.getAttribute(u8"chain");
    if (chainName.size() != 1 || chainName.front() < 'a' || chainName.front() >= 'a' + int(N_CRUMBLING_TILE_CHAINS)) {
        throw xml_error(tag, u8"Unknown crumbling tiles chain id");
    }
    const unsigned chainId = chainName.front() - 'a';

    CrumblingTileChain& chain = tilesetInput.crumblingTiles.at(chainId);

    chain.firstTileId = tag.getAttributeUint8(u8"first-tile");
    chain.firstDelay = tag.getAttributeUint16(u8"first-delay");

    chain.secondTileId = tag.getAttributeUint8(u8"second-tile");

    if (tag.getAttributeBoolean(u8"no-third-tile") == false) {
        chain.secondDelay = tag.getAttributeUint16(u8"second-delay");
        chain.thirdTileId = tag.getAttributeUint8(u8"third-tile");
    }
    else {
        chain.secondDelay = 0xffff;
        chain.thirdTileId = 0;
    }
}

static void writeCrumblingTilesChains(XmlWriter& xml, const std::array<CrumblingTileChain, N_CRUMBLING_TILE_CHAINS>& chains)
{
    static_assert(N_CRUMBLING_TILE_CHAINS < 26);
    std::u8string chainName{ u8"a" };

    for (auto& chain : chains) {
        xml.writeTag(u8"crumbling-tile");

        xml.writeTagAttribute(u8"chain", chainName);

        xml.writeTagAttribute(u8"first-tile", chain.firstTileId);
        xml.writeTagAttribute(u8"first-delay", chain.firstDelay);

        xml.writeTagAttribute(u8"second-tile", chain.secondTileId);

        if (chain.hasThirdTransition()) {
            xml.writeTagAttribute(u8"second-delay", chain.secondDelay);
            xml.writeTagAttribute(u8"third-tile", chain.thirdTileId);
        }
        else {
            xml.writeTagAttribute(u8"no-third-tile", true);
        }

        xml.writeCloseTag();

        chainName.front() += 1;
    }
}

static std::unique_ptr<MetaTileTilesetInput> readMetaTileTilesetInput(XmlReader& xml, const XmlTag& tag)
{
    if (tag.name != u8"metatile-tileset") {
        throw xml_error(xml, u8"Not a Resources file (expected <metatile-tileset> tag)");
    }

    bool readAnimationFramesTag = false;
    bool readScratchpadTag = false;
    bool readTilePrioritiesTag = false;

    auto tilesetInput = std::make_unique<MetaTileTilesetInput>();

    tilesetInput->name = tag.getAttributeId(u8"name");

    while (const auto childTag = xml.parseTag()) {
        if (childTag.name == u8"palette") {
            tilesetInput->palettes.emplace_back(childTag.getAttributeId(u8"name"));
        }
        else if (childTag.name == u8"animation-frames") {
            if (readAnimationFramesTag) {
                throw xml_error(childTag, u8"Only one <animation-frames> tag allowed");
            }
            readAnimationFramesTag = true;

            Resources::readAnimationFramesInput(tilesetInput->animationFrames, xml, childTag);
        }
        else if (childTag.name == u8"tile") {
            readTileProperties(childTag, *tilesetInput);
        }
        else if (childTag.name == u8"scratchpad") {
            if (readScratchpadTag) {
                throw xml_error(childTag, u8"Only one <scratchpad> tag allowed");
            }
            readScratchpadTag = true;

            tilesetInput->scratchpad = readMetaTileGrid(xml, childTag);
        }
        else if (childTag.name == u8"tile-priorities") {
            if (readTilePrioritiesTag) {
                throw xml_error(childTag, u8"Only one <tile-priorities> tag allowed");
            }
            readTilePrioritiesTag = true;

            readTilePriorities(xml, childTag, *tilesetInput);
        }
        else if (childTag.name == u8"crumbling-tile") {
            readCrumblingTilesChain(childTag, *tilesetInput);
        }
        else {
            throw unknown_tag_error(childTag);
        }

        xml.parseCloseTag();
    }

    return tilesetInput;
}

void writeMetaTileTilesetInput(XmlWriter& xml, const MetaTileTilesetInput& input)
{
    xml.writeTag(u8"metatile-tileset");

    xml.writeTagAttribute(u8"name", input.name);

    for (const idstring& pName : input.palettes) {
        xml.writeTag(u8"palette");
        xml.writeTagAttribute(u8"name", pName);
        xml.writeCloseTag();
    }

    writeCrumblingTilesChains(xml, input.crumblingTiles);
    writeTileProperties(xml, input);
    writeTilePriorities(xml, input.tilePriorities);

    Resources::writeAnimationFramesInput(xml, input.animationFrames);

    writeMetaTileGrid(xml, u8"scratchpad", input.scratchpad);

    xml.writeCloseTag();
}

std::unique_ptr<MetaTileTilesetInput> readMetaTileTilesetInput(XmlReader& xml)
{
    try {
        const auto tag = xml.parseTag();
        return readMetaTileTilesetInput(xml, tag);
    }
    catch (const std::exception& ex) {
        throw xml_error(xml, u8"Error loading metatile tileset", ex);
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
        err.addErrorString(convert_old_string(ex.what()));
        return nullptr;
    }
}

void saveMetaTileTilesetInput(const MetaTileTilesetInput& input, const std::filesystem::path& filename)
{
    // utmt files contain a large base64 text block, use a larger buffer.
    XmlWriter xml(filename, u8"untech", 128 * 1024);
    writeMetaTileTilesetInput(xml, input);

    File::atomicWrite(filename, xml.string_view());
}

}
