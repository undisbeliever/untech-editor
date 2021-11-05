/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "metasprite-serializer.h"
#include "animation/serializer.h"

#include "models/common/atomicofstream.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"
#include "models/snes/bit-depth.h"
#include "models/snes/palette-data.hpp"
#include "models/snes/tile-data.h"
#include <cassert>
#include <fstream>
#include <stdexcept>

using namespace UnTech::Xml;

namespace UnTech::MetaSprite {
// Declared in `spriteimporter-serializer.cpp`
extern const EnumMap<TilesetType> tilesetTypeEnumMap;
}

namespace UnTech::MetaSprite::MetaSprite {

const std::string FrameSet::FILE_EXTENSION = "utms";

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

        if (tag.name != "metasprite") {
            throw xml_error(xml, "Expected <metasprite> tag");
        }
        return readFrameSet(xml, tag);
    }
    catch (const std::exception& ex) {
        throw xml_error(xml, "Unable to load MetaSprite FrameSet", ex);
    }
}

void saveFrameSet(const FrameSet& frameSet, const std::filesystem::path& filename)
{
    AtomicOfStream file(filename);
    XmlWriter xml(file, filename, "untech");
    writeFrameSet(xml, frameSet);
    file.commit();
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
        assert(tag.name == "metasprite");
        assert(frameSet.frames.size() == 0);

        frameSet.name = tag.getAttributeId("id");
        frameSet.tilesetType = tag.getAttributeEnum("tilesettype", tilesetTypeEnumMap);

        if (tag.hasAttribute("exportorder")) {
            frameSet.exportOrder = tag.getAttributeId("exportorder");
        }

        while (const auto childTag = xml.parseTag()) {
            if (childTag.name == "frame") {
                readFrame(childTag);
            }
            else if (childTag.name == "smalltileset") {
                readSmallTileset(childTag);
            }
            else if (childTag.name == "largetileset") {
                readLargeTileset(childTag);
            }
            else if (childTag.name == "palette") {
                readPalette(childTag);
            }
            else if (childTag.name == "animation") {
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
            throw xml_error(tag, stringBuilder("Can only have one ", tag.name, " per frame"));
        }

        box.exists = true;
        box.aabb = tag.getAttributeMs8rect();
    }

    inline void readFrame(const XmlTag& tag)
    {
        assert(tag.name == "frame");

        frameSet.frames.insert_back();
        Frame& frame = frameSet.frames.back();

        frame.name = tag.getAttributeId("id");
        frame.spriteOrder = tag.getAttributeUnsigned("order", 0, frame.spriteOrder.MASK);

        while (const auto childTag = xml.parseTag()) {
            if (childTag.name == "object") {
                FrameObject obj;

                std::string sizeStr = childTag.getAttribute("size");
                if (sizeStr == "small") {
                    obj.size = ObjectSize::SMALL;
                }
                else if (sizeStr == "large") {
                    obj.size = ObjectSize::LARGE;
                }
                else {
                    throw xml_error(childTag, "size", "Unknown size");
                }

                obj.location = childTag.getAttributeMs8point();
                obj.tileId = childTag.getAttributeUnsigned("tile");
                obj.hFlip = childTag.getAttributeBoolean("hflip");
                obj.vFlip = childTag.getAttributeBoolean("vflip");

                frame.objects.push_back(obj);
            }
            else if (childTag.name == "actionpoint") {
                ActionPoint ap;

                ap.location = childTag.getAttributeMs8point();
                ap.type = childTag.getAttributeOptionalId("type");

                frame.actionPoints.push_back(ap);
            }
            else if (childTag.name == "tilehitbox") {
                readCollisionBox(childTag, frame.tileHitbox);
            }
            else if (childTag.name == "shield") {
                readCollisionBox(childTag, frame.shield);
            }
            else if (childTag.name == "hitbox") {
                readCollisionBox(childTag, frame.hitbox);
            }
            else if (childTag.name == "hurtbox") {
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
        assert(tag.name == "smalltileset");

        const auto data = xml.parseBase64OfUnknownSize();

        constexpr unsigned smallTileSize = Snes::snesTileSizeForBitdepth(4);
        assert(smallTileSize == 32);
        if ((data.size() % smallTileSize) != 0) {
            throw xml_error(tag, "Small Tileset data must be a multiple of 32 bytes");
        }

        frameSet.smallTileset = Snes::readSnesTileData4bpp(data);
    }

    inline void readLargeTileset(const XmlTag& tag)
    {
        assert(tag.name == "largetileset");

        const auto data = xml.parseBase64OfUnknownSize();

        constexpr unsigned largeTileSize = Snes::snesTileSizeForBitdepth(4) * 4;

        static_assert(largeTileSize == 128, "Bad assumption");
        if ((data.size() % largeTileSize) != 0) {
            throw xml_error(tag, "Large Tileset data must be a multiple of 128 bytes");
        }

        frameSet.largeTileset = Snes::readSnesTileData4bppTile16(data);
    }

    inline void readPalette(const XmlTag& tag)
    {
        assert(tag.name == "palette");

        std::array<uint8_t, 32> data;

        xml.parseBase64ToByteArray(data);

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

static void writeCollisionBox(XmlWriter& xml, const CollisionBox& box, const std::string_view tagName)
{
    if (box.exists) {
        xml.writeTag(tagName);
        xml.writeTagAttributeMs8rect(box.aabb);
        xml.writeCloseTag();
    }
}

inline void writeFrame(XmlWriter& xml, const Frame& frame)
{
    xml.writeTag("frame");

    xml.writeTagAttribute("id", frame.name);
    xml.writeTagAttribute("order", frame.spriteOrder);

    writeCollisionBox(xml, frame.tileHitbox, "tilehitbox");
    writeCollisionBox(xml, frame.shield, "shield");
    writeCollisionBox(xml, frame.hitbox, "hitbox");
    writeCollisionBox(xml, frame.hurtbox, "hurtbox");

    for (const FrameObject& obj : frame.objects) {
        xml.writeTag("object");

        if (obj.size == ObjectSize::SMALL) {
            xml.writeTagAttribute("size", "small");
        }
        else {
            xml.writeTagAttribute("size", "large");
        }

        xml.writeTagAttributeMs8point(obj.location);
        xml.writeTagAttribute("tile", obj.tileId);
        xml.writeTagAttribute("hflip", obj.hFlip);
        xml.writeTagAttribute("vflip", obj.vFlip);

        xml.writeCloseTag();
    }

    for (const ActionPoint& ap : frame.actionPoints) {
        xml.writeTag("actionpoint");

        xml.writeTagAttribute("type", ap.type);
        xml.writeTagAttributeMs8point(ap.location);

        xml.writeCloseTag();
    }

    xml.writeCloseTag();
}

void writeFrameSet(XmlWriter& xml, const FrameSet& frameSet)
{
    xml.writeTag("metasprite");

    xml.writeTagAttribute("id", frameSet.name);
    xml.writeTagAttributeEnum("tilesettype", frameSet.tilesetType, tilesetTypeEnumMap);

    if (frameSet.exportOrder.isValid()) {
        xml.writeTagAttribute("exportorder", frameSet.exportOrder);
    }

    if (frameSet.smallTileset.size() > 0) {
        xml.writeTag("smalltileset");
        xml.writeBase64(Snes::snesTileData4bpp(frameSet.smallTileset));
        xml.writeCloseTag();
    }

    if (frameSet.largeTileset.size() > 0) {
        xml.writeTag("largetileset");
        xml.writeBase64(Snes::snesTileData4bppTile16(frameSet.largeTileset));
        xml.writeCloseTag();
    }

    for (const Snes::Palette4bpp& p : frameSet.palettes) {
        xml.writeTag("palette");
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
