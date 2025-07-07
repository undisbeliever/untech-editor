/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "spriteimporter-serializer.h"
#include "animation/serializer.h"

#include "models/common/file.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"
#include <cassert>
#include <fstream>

using namespace UnTech::Xml;

namespace UnTech::MetaSprite {

extern const EnumMap<ObjectSize> objectSizeEnumMap = {
    { u8"small", ObjectSize::SMALL },
    { u8"large", ObjectSize::LARGE },
};

extern const EnumMap<TilesetType> tilesetTypeEnumMap = {
    { u8"ONE_TILE", TilesetType::ONE_TILE },
    { u8"TWO_TILES", TilesetType::TWO_TILES },
    { u8"ONE_ROW", TilesetType::ONE_ROW },
    { u8"TWO_ROWS", TilesetType::TWO_ROWS },
    { u8"ONE_TILE_FIXED", TilesetType::ONE_TILE_FIXED },
    { u8"TWO_TILES_FIXED", TilesetType::TWO_TILES_FIXED },
    { u8"ONE_ROW_FIXED", TilesetType::ONE_ROW_FIXED },
    { u8"TWO_ROWS_FIXED", TilesetType::TWO_ROWS_FIXED },
};

}

namespace UnTech::MetaSprite::SpriteImporter {

const std::u8string FrameSet::FILE_EXTENSION = u8"utsi";

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

        if (tag.name() != u8"spriteimporter") {
            throw xml_error(xml, u8"Expected <spriteimporter> tag");
        }
        return readFrameSet(xml, tag);
    }
    catch (const std::exception& ex) {
        throw xml_error(xml, u8"Unable to load SpriteImporter FrameSet file", ex);
    }
}

void saveFrameSet(const FrameSet& frameSet, const std::filesystem::path& filename)
{
    XmlWriter xml(filename, u8"untech");
    writeFrameSet(xml, frameSet);

    File::writeFile(filename, xml.string_view());
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

    bool frameSetGridSet{ false };

public:
    inline void readFrameSet(const XmlTag& tag)
    {
        assert(tag.name() == u8"spriteimporter");
        assert(frameSet.frames.size() == 0);

        frameSet.name = tag.getAttributeId(u8"id");
        frameSet.tilesetType = tag.getAttributeEnum(u8"tilesettype", tilesetTypeEnumMap);

        if (tag.hasAttribute(u8"exportorder")) {
            frameSet.exportOrder = tag.getAttributeId(u8"exportorder");
        }

        frameSetGridSet = false;

        if (tag.hasAttribute(u8"image")) {
            frameSet.imageFilename = tag.getAttributeFilename(u8"image");
        }

        if (tag.hasAttribute(u8"transparent")) {
            static_assert(sizeof(unsigned) >= 3, "Unsigned value too small");

            unsigned hex = tag.getAttributeUnsignedHex(u8"transparent");

            frameSet.transparentColor = UnTech::rgba::fromRgba(hex);
            frameSet.transparentColor.alpha = 0xFF;
        }
        else {
            frameSet.transparentColor = rgba(0, 0, 0, 0);
        }

        while (const auto childTag = xml.parseTag()) {
            if (childTag.name() == u8"grid") {
                readFrameSetGrid(childTag);
            }
            else if (childTag.name() == u8"palette") {
                readPalette(childTag);
            }
            else if (childTag.name() == u8"frame") {
                readFrame(childTag);
            }
            else if (childTag.name() == u8"animation") {
                Animation::readAnimation(xml, childTag, frameSet.animations);
            }
            else {
                throw unknown_tag_error(childTag);
            }

            xml.parseCloseTag();
        }
    }

private:
    inline void readFrameSetGrid(const XmlTag& tag)
    {
        assert(tag.name() == u8"grid");

        frameSet.grid.frameSize = tag.getAttributeUsize(u8"width", u8"height");
        frameSet.grid.offset = tag.getAttributeUpoint(u8"xoffset", u8"yoffset");
        frameSet.grid.padding = tag.getAttributeUpoint(u8"xpadding", u8"ypadding");
        frameSet.grid.origin = tag.getAttributeUpoint(u8"xorigin", u8"yorigin");

        frameSetGridSet = true;
    }

    static void readCollisionBox(const XmlTag& tag, CollisionBox& box)
    {
        if (box.exists) {
            throw xml_error(tag, stringBuilder(u8"Can only have one ", tag.name(), u8" per frame"));
        }

        box.exists = true;
        box.aabb = tag.getAttributeUrect();
    }

    inline void readFrame(const XmlTag& tag)
    {
        assert(tag.name() == u8"frame");

        frameSet.frames.insert_back();
        Frame& frame = frameSet.frames.back();

        frame.name = tag.getAttributeId(u8"id");

        if (tag.hasAttribute(u8"order")) {
            frame.spriteOrder = tag.getAttributeUnsigned(u8"order", 0, frame.spriteOrder.MASK);
        }

        while (const auto childTag = xml.parseTag()) {
            if (childTag.name() == u8"gridlocation") {
                frame.gridLocation = childTag.getAttributeUpoint();
            }
            else if (childTag.name() == u8"location") {
                frame.locationOverride = childTag.getAttributeUrect();
            }
            else if (childTag.name() == u8"origin") {
                frame.originOverride = childTag.getAttributeUpoint();
            }
            else if (childTag.name() == u8"object") {
                FrameObject obj;

                obj.size = childTag.getAttributeEnum(u8"size", objectSizeEnumMap);
                obj.location = childTag.getAttributeUpoint();

                frame.objects.push_back(obj);
            }

            else if (childTag.name() == u8"actionpoint") {
                ActionPoint ap;

                ap.location = childTag.getAttributeUpoint();
                ap.type = childTag.getAttributeOptionalId(u8"type");

                frame.actionPoints.push_back(ap);
            }
            else if (childTag.name() == u8"tilehitbox") {
                readCollisionBox(childTag, frame.tileHitbox);
            }
            else if (childTag.name() == u8"shield") {
                readCollisionBox(childTag, frame.shield);
            }
            else if (childTag.name() == u8"hitbox") {
                readCollisionBox(childTag, frame.hitbox);
            }
            else if (childTag.name() == u8"hurtbox") {
                readCollisionBox(childTag, frame.hurtbox);
            }
            else {
                throw unknown_tag_error(childTag);
            }

            xml.parseCloseTag();
        }
    }

    inline void readPalette(const XmlTag& tag)
    {
        assert(tag.name() == u8"palette");

        auto& palette = frameSet.palette;

        if (palette.usesUserSuppliedPalette()) {
            throw xml_error(tag, u8"Only one palette tag allowed per frameset");
        }

        if (tag.hasAttribute(u8"position")) {
            palette.position = tag.getAttributeEnum(u8"position", UserSuppliedPalette::positionEnumMap);
        }
        else {
            // old behaviour
            palette.position = UserSuppliedPalette::Position::BOTTOM_LEFT;
        }
        palette.nPalettes = tag.getAttributeUnsigned(u8"npalettes", 1);
        palette.colorSize = tag.getAttributeUnsigned(u8"colorsize", 1);
    }
};

std::unique_ptr<FrameSet> readFrameSet(XmlReader& xml, const XmlTag& tag)
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
        xml.writeTagAttributeUrect(box.aabb);
        xml.writeCloseTag();
    }
}

inline void writeFrame(XmlWriter& xml, const Frame& frame)
{
    xml.writeTag(u8"frame");

    xml.writeTagAttribute(u8"id", frame.name);
    xml.writeTagAttribute(u8"order", frame.spriteOrder);

    if (frame.locationOverride) {
        xml.writeTag(u8"location");
        xml.writeTagAttributeUrect(frame.locationOverride.value());
        xml.writeCloseTag();
    }

    xml.writeTag(u8"gridlocation");
    xml.writeTagAttributeUpoint(frame.gridLocation);
    xml.writeCloseTag();

    if (frame.originOverride) {
        xml.writeTag(u8"origin");
        xml.writeTagAttributeUpoint(frame.originOverride.value());
        xml.writeCloseTag();
    }

    writeCollisionBox(xml, frame.tileHitbox, u8"tilehitbox");
    writeCollisionBox(xml, frame.shield, u8"shield");
    writeCollisionBox(xml, frame.hitbox, u8"hitbox");
    writeCollisionBox(xml, frame.hurtbox, u8"hurtbox");

    for (const FrameObject& obj : frame.objects) {
        xml.writeTag(u8"object");

        xml.writeTagAttributeEnum(u8"size", obj.size, objectSizeEnumMap);
        xml.writeTagAttributeUpoint(obj.location);

        xml.writeCloseTag();
    }

    for (const ActionPoint& ap : frame.actionPoints) {
        xml.writeTag(u8"actionpoint");

        xml.writeTagAttribute(u8"type", ap.type);
        xml.writeTagAttributeUpoint(ap.location);

        xml.writeCloseTag();
    }

    xml.writeCloseTag();
}

inline void writeFrameSetGrid(XmlWriter& xml, const FrameSetGrid& grid)
{
    xml.writeTag(u8"grid");

    xml.writeTagAttributeUsize(grid.frameSize, u8"width", u8"height");
    xml.writeTagAttributeUpoint(grid.offset, u8"xoffset", u8"yoffset");
    xml.writeTagAttributeUpoint(grid.padding, u8"xpadding", u8"ypadding");
    xml.writeTagAttributeUpoint(grid.origin, u8"xorigin", u8"yorigin");

    xml.writeCloseTag();
}

void writeFrameSet(XmlWriter& xml, const FrameSet& frameSet)
{
    xml.writeTag(u8"spriteimporter");

    xml.writeTagAttribute(u8"id", frameSet.name);
    xml.writeTagAttributeEnum(u8"tilesettype", frameSet.tilesetType, tilesetTypeEnumMap);

    if (frameSet.exportOrder.isValid()) {
        xml.writeTagAttribute(u8"exportorder", frameSet.exportOrder);
    }
    if (!frameSet.imageFilename.empty()) {
        xml.writeTagAttributeFilename(u8"image", frameSet.imageFilename);
    }

    if (frameSet.transparentColor.alpha == 0xff) {
        static_assert(sizeof(unsigned) >= 3, "Unsigned value too small");

        xml.writeTagAttributeHex6(u8"transparent", frameSet.transparentColor.rgb());
    }

    writeFrameSetGrid(xml, frameSet.grid);

    if (frameSet.palette.usesUserSuppliedPalette()) {
        xml.writeTag(u8"palette");
        xml.writeTagAttributeEnum(u8"position", frameSet.palette.position, UserSuppliedPalette::positionEnumMap);
        xml.writeTagAttribute(u8"npalettes", frameSet.palette.nPalettes);
        xml.writeTagAttribute(u8"colorsize", frameSet.palette.colorSize);
        xml.writeCloseTag();
    }

    for (const auto& frame : frameSet.frames) {
        writeFrame(xml, frame);
    }

    Animation::writeAnimations(xml, frameSet.animations);

    xml.writeCloseTag();
}

}
