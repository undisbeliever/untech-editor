/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "metasprite-serializer.h"
#include "animation/serializer.h"

#include "models/common/atomicofstream.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"
#include <cassert>
#include <fstream>
#include <stdexcept>

using namespace UnTech;
using namespace UnTech::Xml;

namespace UnTech {
namespace MetaSprite {
namespace MetaSprite {

const std::string FrameSet::FILE_EXTENSION = "utms";

std::unique_ptr<FrameSet> readFrameSet(XmlReader& xml, const XmlTag* tag);

std::unique_ptr<FrameSet> loadFrameSet(const std::string& filename)
{
    auto xml = XmlReader::fromFile(filename);
    return readFrameSet(*xml);
}

std::unique_ptr<FrameSet> readFrameSet(XmlReader& xml)
{
    try {
        std::unique_ptr<XmlTag> tag = xml.parseTag();

        if (tag == nullptr || tag->name != "metasprite") {
            throw xml_error(xml, "Expected <metasprite> tag");
        }
        return readFrameSet(xml, tag.get());
    }
    catch (const std::exception& ex) {
        throw xml_error(xml, "Unable to load MetaSprite FrameSet", ex);
    }
}

void saveFrameSet(const FrameSet& frameSet, const std::string& filename)
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
    inline void readFrameSet(const XmlTag* tag)
    {
        assert(tag->name == "metasprite");
        assert(frameSet.frames.size() == 0);

        frameSet.name = tag->getAttributeId("id");
        frameSet.tilesetType = tag->getAttributeEnum<TilesetType>("tilesettype");

        if (tag->hasAttribute("exportorder")) {
            frameSet.exportOrder = tag->getAttributeId("exportorder");
        }

        std::unique_ptr<XmlTag> childTag;
        while ((childTag = xml.parseTag())) {
            if (childTag->name == "frame") {
                readFrame(childTag.get());
            }
            else if (childTag->name == "smalltileset") {
                readSmallTileset(childTag.get());
            }
            else if (childTag->name == "largetileset") {
                readLargeTileset(childTag.get());
            }
            else if (childTag->name == "palette") {
                readPalette(childTag.get());
            }
            else if (childTag->name == "animation") {
                Animation::readAnimation(xml, childTag.get(), frameSet.animations);
            }
            else {
                throw unknown_tag_error(*childTag);
            }

            xml.parseCloseTag();
        }
    }

private:
    inline void readFrame(const XmlTag* tag)
    {
        assert(tag->name == "frame");

        frameSet.frames.insert_back();
        Frame& frame = frameSet.frames.back();

        frame.name = tag->getAttributeId("id");
        frame.spriteOrder = tag->getAttributeUnsigned("order", 0, frame.spriteOrder.MASK);
        frame.solid = false;

        std::unique_ptr<XmlTag> childTag;

        while ((childTag = xml.parseTag())) {
            if (childTag->name == "object") {
                FrameObject obj;

                std::string sizeStr = childTag->getAttribute("size");
                if (sizeStr == "small") {
                    obj.size = ObjectSize::SMALL;
                }
                else if (sizeStr == "large") {
                    obj.size = ObjectSize::LARGE;
                }
                else {
                    throw xml_error(*childTag, "size", "Unknown size");
                }

                obj.location = childTag->getAttributeMs8point();
                obj.tileId = childTag->getAttributeUnsigned("tile");
                obj.hFlip = childTag->getAttributeBoolean("hflip");
                obj.vFlip = childTag->getAttributeBoolean("vflip");

                frame.objects.push_back(obj);
            }

            else if (childTag->name == "actionpoint") {
                ActionPoint ap;

                ap.location = childTag->getAttributeMs8point();
                ap.parameter = childTag->getAttributeClamped<ActionPointParameter>("parameter");

                frame.actionPoints.push_back(ap);
            }

            else if (childTag->name == "entityhitbox") {
                EntityHitbox eh;

                eh.aabb = childTag->getAttributeMs8rect();
                eh.hitboxType = EntityHitboxType::from_string(childTag->getAttribute("type"));

                frame.entityHitboxes.push_back(eh);
            }

            else if (childTag->name == "tilehitbox") {
                if (frame.solid == true) {
                    throw xml_error(*childTag, "Can only have one tilehitbox per frame");
                }
                frame.tileHitbox = childTag->getAttributeMs8rect();
                frame.solid = true;
            }

            else {
                throw unknown_tag_error(*childTag);
            }

            xml.parseCloseTag();
        }
    }

    inline void readSmallTileset(const XmlTag* tag)
    {
        assert(tag->name == "smalltileset");

        const auto data = xml.parseBase64();

        assert(frameSet.smallTileset.snesTileSize() == 32);
        if ((data.size() % 32) != 0) {
            throw xml_error(*tag, "Small Tileset data must be a multiple of 32 bytes");
        }

        frameSet.smallTileset.readSnesData(data);
    }

    inline void readLargeTileset(const XmlTag* tag)
    {
        assert(tag->name == "largetileset");

        const auto data = xml.parseBase64();

        static_assert(Snes::TilesetTile16::SNES_TILE_SIZE == 128, "Bad assumption");
        if ((data.size() % 128) != 0) {
            throw xml_error(*tag, "Large Tileset data must be a multiple of 128 bytes");
        }

        frameSet.largeTileset.readSnesData(data);
    }

    inline void readPalette(const XmlTag* tag)
    {
        const static unsigned N_COLORS = 16;

        assert(tag->name == "palette");

        const auto data = xml.parseBase64();

        static_assert(N_COLORS * 2 == 32, "Bad assumption");
        if (data.size() != 32) {
            throw xml_error(*tag, "Palette data must contain 32 bytes");
        }

        frameSet.palettes.emplace_back(data);
    }
};

std::unique_ptr<FrameSet> readFrameSet(Xml::XmlReader& xml, const Xml::XmlTag* tag)
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

inline void writeFrame(XmlWriter& xml, const Frame& frame)
{
    xml.writeTag("frame");

    xml.writeTagAttribute("id", frame.name);
    xml.writeTagAttribute("order", frame.spriteOrder);

    if (frame.solid) {
        xml.writeTag("tilehitbox");

        xml.writeTagAttributeMs8rect(frame.tileHitbox);

        xml.writeCloseTag();
    }

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

        xml.writeTagAttribute("parameter", ap.parameter);
        xml.writeTagAttributeMs8point(ap.location);

        xml.writeCloseTag();
    }

    for (const EntityHitbox& eh : frame.entityHitboxes) {
        xml.writeTag("entityhitbox");

        xml.writeTagAttribute("type", eh.hitboxType.to_string());
        xml.writeTagAttributeMs8rect(eh.aabb);

        xml.writeCloseTag();
    }

    xml.writeCloseTag();
}

void writeFrameSet(XmlWriter& xml, const FrameSet& frameSet)
{
    xml.writeTag("metasprite");

    xml.writeTagAttribute("id", frameSet.name);
    xml.writeTagAttributeEnum("tilesettype", frameSet.tilesetType);

    if (frameSet.exportOrder.isValid()) {
        xml.writeTagAttribute("exportorder", frameSet.exportOrder);
    }

    if (frameSet.smallTileset.size() > 0) {
        xml.writeTag("smalltileset");
        xml.writeBase64(frameSet.smallTileset.snesData());
        xml.writeCloseTag();
    }

    if (frameSet.largeTileset.size() > 0) {
        xml.writeTag("largetileset");
        xml.writeBase64(frameSet.largeTileset.snesData());
        xml.writeCloseTag();
    }

    for (const Snes::Palette4bpp& p : frameSet.palettes) {
        xml.writeTag("palette");
        xml.writeBase64(p.paletteData());
        xml.writeCloseTag();
    }

    for (const auto& frame : frameSet.frames) {
        writeFrame(xml, frame);
    }

    Animation::writeAnimations(xml, frameSet.animations);

    xml.writeCloseTag();
}
}
}
}
