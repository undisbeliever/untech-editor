/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "spriteimporter-serializer.h"
#include "animation/serializer.h"

#include "models/common/atomicofstream.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"
#include <cassert>
#include <fstream>

using namespace UnTech::Xml;

namespace UnTech::MetaSprite {

extern const EnumMap<ObjectSize> objectSizeEnumMap = {
    { "small", ObjectSize::SMALL },
    { "large", ObjectSize::LARGE },
};

extern const EnumMap<TilesetType> tilesetTypeEnumMap = {
    { "ONE_TILE", TilesetType::ONE_TILE },
    { "TWO_TILES", TilesetType::TWO_TILES },
    { "ONE_ROW", TilesetType::ONE_ROW },
    { "TWO_ROWS", TilesetType::TWO_ROWS },
    { "ONE_TILE_FIXED", TilesetType::ONE_TILE_FIXED },
    { "TWO_TILES_FIXED", TilesetType::TWO_TILES_FIXED },
    { "ONE_ROW_FIXED", TilesetType::ONE_ROW_FIXED },
    { "TWO_ROWS_FIXED", TilesetType::TWO_ROWS_FIXED },
};

}

namespace UnTech::MetaSprite::SpriteImporter {

const std::string FrameSet::FILE_EXTENSION = "utsi";

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

        if (tag.name != "spriteimporter") {
            throw xml_error(xml, "Expected <spriteimporter> tag");
        }
        return readFrameSet(xml, tag);
    }
    catch (const std::exception& ex) {
        throw xml_error(xml, "Unable to load SpriteImporter FrameSet file", ex);
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
        , frameSetGridSet(false)
    {
    }

private:
    FrameSet& frameSet;
    XmlReader& xml;

    bool frameSetGridSet;

public:
    inline void readFrameSet(const XmlTag& tag)
    {
        assert(tag.name == "spriteimporter");
        assert(frameSet.frames.size() == 0);

        frameSet.name = tag.getAttributeId("id");
        frameSet.tilesetType = tag.getAttributeEnum("tilesettype", tilesetTypeEnumMap);

        if (tag.hasAttribute("exportorder")) {
            frameSet.exportOrder = tag.getAttributeId("exportorder");
        }

        frameSetGridSet = false;

        if (tag.hasAttribute("image")) {
            frameSet.imageFilename = tag.getAttributeFilename("image");
        }

        if (tag.hasAttribute("transparent")) {
            static_assert(sizeof(unsigned) >= 3, "Unsigned value too small");

            unsigned hex = tag.getAttributeUnsignedHex("transparent");

            frameSet.transparentColor = UnTech::rgba::fromRgba(hex);
            frameSet.transparentColor.alpha = 0xFF;
        }
        else {
            frameSet.transparentColor = rgba(0, 0, 0, 0);
        }

        while (const auto childTag = xml.parseTag()) {
            if (childTag.name == "grid") {
                readFrameSetGrid(childTag);
            }
            else if (childTag.name == "palette") {
                readPalette(childTag);
            }
            else if (childTag.name == "frame") {
                readFrame(childTag);
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
    inline void readFrameSetGrid(const XmlTag& tag)
    {
        assert(tag.name == "grid");

        frameSet.grid.frameSize = tag.getAttributeUsize("width", "height");
        frameSet.grid.offset = tag.getAttributeUpoint("xoffset", "yoffset");
        frameSet.grid.padding = tag.getAttributeUpoint("xpadding", "ypadding");
        frameSet.grid.origin = tag.getAttributeUpoint("xorigin", "yorigin");

        frameSetGridSet = true;
    }

    static void readCollisionBox(const XmlTag& tag, CollisionBox& box)
    {
        if (box.exists) {
            throw xml_error(tag, stringBuilder("Can only have one ", tag.name, " per frame"));
        }

        box.exists = true;
        box.aabb = tag.getAttributeUrect();
    }

    inline void readFrame(const XmlTag& tag)
    {
        assert(tag.name == "frame");

        frameSet.frames.insert_back();
        Frame& frame = frameSet.frames.back();

        frame.name = tag.getAttributeId("id");

        if (tag.hasAttribute("order")) {
            frame.spriteOrder = tag.getAttributeUnsigned("order", 0, frame.spriteOrder.MASK);
        }

        if (const auto childTag = xml.parseTag()) {
            if (childTag.name == "location") {
                frame.location.aabb = childTag.getAttributeUrect();
                frame.location.useGridLocation = false;
            }
            else if (childTag.name == "gridlocation") {
                if (frameSetGridSet == false) {
                    throw xml_error(childTag, "Frameset grid is not set.");
                }

                frame.location.gridLocation = childTag.getAttributeUpoint();
                frame.location.useGridLocation = true;
            }
            else {
                throw xml_error(childTag, "location or gridlocation tag must be the first child of frame");
            }
            xml.parseCloseTag();
        }
        else {
            throw xml_error(tag, "location or gridlocation tag must be the first child of frame");
        }

        frame.location.useGridOrigin = true;
        frame.location.update(frameSet.grid, frame);

        while (const auto childTag = xml.parseTag()) {
            if (childTag.name == "object") {
                FrameObject obj;

                obj.size = childTag.getAttributeEnum("size", objectSizeEnumMap);
                obj.location = childTag.getAttributeUpoint();

                frame.objects.push_back(obj);
            }

            else if (childTag.name == "actionpoint") {
                ActionPoint ap;

                ap.location = childTag.getAttributeUpoint();
                ap.type = childTag.getAttributeOptionalId("type");

                frame.actionPoints.push_back(ap);
            }

            else if (childTag.name == "origin") {
                if (frame.location.useGridOrigin == false) {
                    throw xml_error(childTag, "Can only have one origin per frame");
                }
                frame.location.useGridOrigin = false;
                frame.location.origin = childTag.getAttributeUpoint();
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

    inline void readPalette(const XmlTag& tag)
    {
        assert(tag.name == "palette");

        auto& palette = frameSet.palette;

        if (palette.usesUserSuppliedPalette()) {
            throw xml_error(tag, "Only one palette tag allowed per frameset");
        }

        if (tag.hasAttribute("position")) {
            palette.position = tag.getAttributeEnum("position", UserSuppliedPalette::positionEnumMap);
        }
        else {
            // old behaviour
            palette.position = UserSuppliedPalette::Position::BOTTOM_LEFT;
        }
        palette.nPalettes = tag.getAttributeUnsigned("npalettes", 1);
        palette.colorSize = tag.getAttributeUnsigned("colorsize", 1);
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

static void writeCollisionBox(XmlWriter& xml, const CollisionBox& box, const std::string_view tagName)
{
    if (box.exists) {
        xml.writeTag(tagName);
        xml.writeTagAttributeUrect(box.aabb);
        xml.writeCloseTag();
    }
}

inline void writeFrame(XmlWriter& xml, const Frame& frame)
{
    xml.writeTag("frame");

    xml.writeTagAttribute("id", frame.name);
    xml.writeTagAttribute("order", frame.spriteOrder);

    if (frame.location.useGridLocation) {
        xml.writeTag("gridlocation");
        xml.writeTagAttributeUpoint(frame.location.gridLocation);
        xml.writeCloseTag();
    }
    else {
        xml.writeTag("location");
        xml.writeTagAttributeUrect(frame.location.aabb);
        xml.writeCloseTag();
    }

    if (frame.location.useGridOrigin == false) {
        xml.writeTag("origin");
        xml.writeTagAttributeUpoint(frame.location.origin);
        xml.writeCloseTag();
    }

    writeCollisionBox(xml, frame.tileHitbox, "tilehitbox");
    writeCollisionBox(xml, frame.shield, "shield");
    writeCollisionBox(xml, frame.hitbox, "hitbox");
    writeCollisionBox(xml, frame.hurtbox, "hurtbox");

    for (const FrameObject& obj : frame.objects) {
        xml.writeTag("object");

        xml.writeTagAttributeEnum("size", obj.size, objectSizeEnumMap);
        xml.writeTagAttributeUpoint(obj.location);

        xml.writeCloseTag();
    }

    for (const ActionPoint& ap : frame.actionPoints) {
        xml.writeTag("actionpoint");

        xml.writeTagAttribute("type", ap.type);
        xml.writeTagAttributeUpoint(ap.location);

        xml.writeCloseTag();
    }

    xml.writeCloseTag();
}

inline void writeFrameSetGrid(XmlWriter& xml, const FrameSetGrid& grid)
{
    xml.writeTag("grid");

    xml.writeTagAttributeUsize(grid.frameSize, "width", "height");
    xml.writeTagAttributeUpoint(grid.offset, "xoffset", "yoffset");
    xml.writeTagAttributeUpoint(grid.padding, "xpadding", "ypadding");
    xml.writeTagAttributeUpoint(grid.origin, "xorigin", "yorigin");

    xml.writeCloseTag();
}

void writeFrameSet(XmlWriter& xml, const FrameSet& frameSet)
{
    xml.writeTag("spriteimporter");

    xml.writeTagAttribute("id", frameSet.name);
    xml.writeTagAttributeEnum("tilesettype", frameSet.tilesetType, tilesetTypeEnumMap);

    if (frameSet.exportOrder.isValid()) {
        xml.writeTagAttribute("exportorder", frameSet.exportOrder);
    }
    if (!frameSet.imageFilename.empty()) {
        xml.writeTagAttributeFilename("image", frameSet.imageFilename);
    }

    if (frameSet.transparentColor.alpha == 0xff) {
        static_assert(sizeof(unsigned) >= 3, "Unsigned value too small");

        unsigned rgb = frameSet.transparentColor.rgb();
        xml.writeTagAttributeHex("transparent", rgb, 6);
    }

    writeFrameSetGrid(xml, frameSet.grid);

    if (frameSet.palette.usesUserSuppliedPalette()) {
        xml.writeTag("palette");
        xml.writeTagAttributeEnum("position", frameSet.palette.position, UserSuppliedPalette::positionEnumMap);
        xml.writeTagAttribute("npalettes", frameSet.palette.nPalettes);
        xml.writeTagAttribute("colorsize", frameSet.palette.colorSize);
        xml.writeCloseTag();
    }

    for (const auto& frame : frameSet.frames) {
        writeFrame(xml, frame);
    }

    Animation::writeAnimations(xml, frameSet.animations);

    xml.writeCloseTag();
}

}
