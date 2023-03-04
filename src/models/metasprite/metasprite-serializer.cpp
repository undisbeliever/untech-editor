/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "metasprite-serializer.h"
#include "animation/serializer.h"

#include "models/common/file.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"
#include "models/snes/bit-depth.h"
#include "models/snes/palette-data.hpp"
#include "models/snes/tile-data.h"
#include <cassert>
#include <fstream>

using namespace UnTech::Xml;

namespace UnTech::MetaSprite {
// defined in `spriteimporter-serializer.cpp`
extern const EnumMap<ObjectSize> objectSizeEnumMap;

// Declared in `spriteimporter-serializer.cpp`
extern const EnumMap<TilesetType> tilesetTypeEnumMap;
}

namespace UnTech::MetaSprite::MetaSprite {

const std::u8string FrameSet::FILE_EXTENSION = u8"utms";

std::unique_ptr<FrameSet> readFrameSet(XmlReader& xml, const XmlTag& tag);

std::unique_ptr<FrameSet> loadFrameSet(const std::filesystem::path& filename)
{
    auto xml = XmlReader::fromFile(filename);
    return readFrameSet(*xml);
}

std::unique_ptr<FrameSet> readFrameSet(XmlReader& xml)
{
    try {
        const auto tag = xml.parseTag();

        if (tag.name != u8"metasprite") {
            throw xml_error(xml, u8"Expected <metasprite> tag");
        }
        return readFrameSet(xml, tag);
    }
    catch (const std::exception& ex) {
        throw xml_error(xml, u8"Unable to load MetaSprite FrameSet", ex);
    }
}

void saveFrameSet(const FrameSet& frameSet, const std::filesystem::path& filename)
{
    // utms files contain base64 text, use a larger buffer.
    XmlWriter xml(filename, u8"untech", 32 * 1024);
    writeFrameSet(xml, frameSet);

    File::atomicWrite(filename, xml.string_view());
}

/*
 * FRAME SET READER
 * ================
 */

struct FrameSetReader {
    FrameSetReader(FrameSet& frameSet, XmlReader& xml)
        : frameSet(frameSet)
        , xml(xml)
    {
    }

private:
    FrameSet& frameSet;
    XmlReader& xml;

public:
    inline void readFrameSet(const XmlTag& tag)
    {
        assert(tag.name == u8"metasprite");
        assert(frameSet.frames.size() == 0);

        frameSet.name = tag.getAttributeId(u8"id");
        frameSet.tilesetType = tag.getAttributeEnum(u8"tilesettype", tilesetTypeEnumMap);

        if (tag.hasAttribute(u8"exportorder")) {
            frameSet.exportOrder = tag.getAttributeId(u8"exportorder");
        }

        while (const auto childTag = xml.parseTag()) {
            if (childTag.name == u8"frame") {
                readFrame(childTag);
            }
            else if (childTag.name == u8"smalltileset") {
                readSmallTileset(childTag);
            }
            else if (childTag.name == u8"largetileset") {
                readLargeTileset(childTag);
            }
            else if (childTag.name == u8"palette") {
                readPalette(childTag);
            }
            else if (childTag.name == u8"animation") {
                Animation::readAnimation(xml, childTag, frameSet.animations);
            }
            else {
                throw unknown_tag_error(childTag);
            }

            xml.parseCloseTag();
        }
    }

private:
    static void readCollisionBox(const XmlTag& tag, CollisionBox& box)
    {
        if (box.exists) {
            throw xml_error(tag, stringBuilder(u8"Can only have one ", tag.name, u8" per frame"));
        }

        box.exists = true;
        box.aabb = tag.getAttributeMs8rect();
    }

    inline void readFrame(const XmlTag& tag)
    {
        assert(tag.name == u8"frame");

        frameSet.frames.insert_back();
        Frame& frame = frameSet.frames.back();

        frame.name = tag.getAttributeId(u8"id");
        frame.spriteOrder = tag.getAttributeUnsigned(u8"order", 0, frame.spriteOrder.MASK);

        while (const auto childTag = xml.parseTag()) {
            if (childTag.name == u8"object") {
                FrameObject obj;

                obj.size = childTag.getAttributeEnum(u8"size", objectSizeEnumMap);
                obj.location = childTag.getAttributeMs8point();
                obj.tileId = childTag.getAttributeUnsigned(u8"tile");
                obj.hFlip = childTag.getAttributeBoolean(u8"hflip");
                obj.vFlip = childTag.getAttributeBoolean(u8"vflip");

                frame.objects.push_back(obj);
            }
            else if (childTag.name == u8"actionpoint") {
                ActionPoint ap;

                ap.location = childTag.getAttributeMs8point();
                ap.type = childTag.getAttributeOptionalId(u8"type");

                frame.actionPoints.push_back(ap);
            }
            else if (childTag.name == u8"tilehitbox") {
                readCollisionBox(childTag, frame.tileHitbox);
            }
            else if (childTag.name == u8"shield") {
                readCollisionBox(childTag, frame.shield);
            }
            else if (childTag.name == u8"hitbox") {
                readCollisionBox(childTag, frame.hitbox);
            }
            else if (childTag.name == u8"hurtbox") {
                readCollisionBox(childTag, frame.hurtbox);
            }
            else {
                throw unknown_tag_error(childTag);
            }

            xml.parseCloseTag();
        }
    }

    inline void readSmallTileset(const XmlTag& tag)
    {
        assert(tag.name == u8"smalltileset");

        const auto data = xml.parseBase64OfUnknownSize();

        constexpr unsigned smallTileSize = Snes::snesTileSizeForBitdepth(Snes::BitDepth::BD_4BPP);
        assert(smallTileSize == 32);
        if ((data.size() % smallTileSize) != 0) {
            throw xml_error(tag, u8"Small Tileset data must be a multiple of 32 bytes");
        }

        frameSet.smallTileset = Snes::readSnesTileData4bpp(data);
    }

    inline void readLargeTileset(const XmlTag& tag)
    {
        assert(tag.name == u8"largetileset");

        const auto data = xml.parseBase64OfUnknownSize();

        constexpr unsigned largeTileSize = Snes::snesTileSizeForBitdepth(Snes::BitDepth::BD_4BPP) * 4;

        static_assert(largeTileSize == 128, u8"Bad assumption");
        if ((data.size() % largeTileSize) != 0) {
            throw xml_error(tag, u8"Large Tileset data must be a multiple of 128 bytes");
        }

        frameSet.largeTileset = Snes::readSnesTileData4bppTile16(data);
    }

    inline void readPalette(const XmlTag& tag)
    {
        assert(tag.name == u8"palette");

        std::array<uint8_t, 32> data{};

        xml.parseBase64ToFixedSizeBuffer(data);

        frameSet.palettes.push_back(Snes::readSnesPaletteData(data));
    }
};

std::unique_ptr<FrameSet> readFrameSet(Xml::XmlReader& xml, const Xml::XmlTag& tag)
{
    auto frameSet = std::make_unique<FrameSet>();

    FrameSetReader reader(*frameSet.get(), xml);
    reader.readFrameSet(tag);

    return frameSet;
}

/*
 * FRAME SET WRITER
 * ================
 */

static void writeCollisionBox(XmlWriter& xml, const CollisionBox& box, const std::u8string_view tagName)
{
    if (box.exists) {
        xml.writeTag(tagName);
        xml.writeTagAttributeMs8rect(box.aabb);
        xml.writeCloseTag();
    }
}

inline void writeFrame(XmlWriter& xml, const Frame& frame)
{
    xml.writeTag(u8"frame");

    xml.writeTagAttribute(u8"id", frame.name);
    xml.writeTagAttribute(u8"order", frame.spriteOrder);

    writeCollisionBox(xml, frame.tileHitbox, u8"tilehitbox");
    writeCollisionBox(xml, frame.shield, u8"shield");
    writeCollisionBox(xml, frame.hitbox, u8"hitbox");
    writeCollisionBox(xml, frame.hurtbox, u8"hurtbox");

    for (const FrameObject& obj : frame.objects) {
        xml.writeTag(u8"object");

        xml.writeTagAttributeEnum(u8"size", obj.size, objectSizeEnumMap);
        xml.writeTagAttributeMs8point(obj.location);
        xml.writeTagAttribute(u8"tile", obj.tileId);
        xml.writeTagAttribute(u8"hflip", obj.hFlip);
        xml.writeTagAttribute(u8"vflip", obj.vFlip);

        xml.writeCloseTag();
    }

    for (const ActionPoint& ap : frame.actionPoints) {
        xml.writeTag(u8"actionpoint");

        xml.writeTagAttribute(u8"type", ap.type);
        xml.writeTagAttributeMs8point(ap.location);

        xml.writeCloseTag();
    }

    xml.writeCloseTag();
}

void writeFrameSet(XmlWriter& xml, const FrameSet& frameSet)
{
    xml.writeTag(u8"metasprite");

    xml.writeTagAttribute(u8"id", frameSet.name);
    xml.writeTagAttributeEnum(u8"tilesettype", frameSet.tilesetType, tilesetTypeEnumMap);

    if (frameSet.exportOrder.isValid()) {
        xml.writeTagAttribute(u8"exportorder", frameSet.exportOrder);
    }

    if (frameSet.smallTileset.size() > 0) {
        xml.writeTag(u8"smalltileset");
        xml.writeBase64(Snes::snesTileData4bpp(frameSet.smallTileset));
        xml.writeCloseTag();
    }

    if (frameSet.largeTileset.size() > 0) {
        xml.writeTag(u8"largetileset");
        xml.writeBase64(Snes::snesTileData4bppTile16(frameSet.largeTileset));
        xml.writeCloseTag();
    }

    for (const Snes::Palette4bpp& p : frameSet.palettes) {
        xml.writeTag(u8"palette");
        xml.writeBase64(snesPaletteData(p));
        xml.writeCloseTag();
    }

    for (const auto& frame : frameSet.frames) {
        writeFrame(xml, frame);
    }

    Animation::writeAnimations(xml, frameSet.animations);

    xml.writeCloseTag();
}

}
