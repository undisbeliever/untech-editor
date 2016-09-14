#include "metasprite-serializer.h"
#include "animation/serializer.h"

#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"
#include "models/snes/palette.hpp"
#include "models/snes/tileset.hpp"
#include <cassert>
#include <fstream>
#include <stdexcept>

using namespace UnTech;
using namespace UnTech::Xml;

namespace UnTech {
namespace MetaSprite {
namespace MetaSprite {

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

        frameSet.tilesetType = tag->getAttributeSimpleClass<TilesetType>("tilesettype");

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
            else if (childTag->name == "exportorder") {
                // ::TODO readExportOrder (from export order module)::
            }
            else if (childTag->name == "animation") {
                Animation::readAnimation(xml, childTag.get(), frameSet.animations);
            }
            else {
                throw childTag->buildUnknownTagError();
            }

            xml.parseCloseTag();
        }
    }

private:
    inline void readFrame(const XmlTag* tag)
    {
        assert(tag->name == "frame");

        std::string id = tag->getAttributeUniqueId("id", frameSet.frames);
        Frame& frame = frameSet.frames[id];
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
                    throw childTag->buildError("size", "Unknown size");
                }

                obj.location = childTag->getAttributeMs8point();
                obj.tileId = childTag->getAttributeUnsigned("tile");
                obj.order = childTag->getAttributeUnsigned("order", 0, obj.order.MASK);
                obj.hFlip = childTag->getAttributeBoolean("hflip");
                obj.vFlip = childTag->getAttributeBoolean("vflip");

                frame.objects.push_back(obj);
            }

            else if (childTag->name == "actionpoint") {
                ActionPoint ap;

                ap.location = childTag->getAttributeMs8point();
                ap.parameter = childTag->getAttributeClampedInteger<ActionPointParameter>("parameter");

                frame.actionPoints.push_back(ap);
            }

            else if (childTag->name == "entityhitbox") {
                EntityHitbox eh;

                eh.aabb = childTag->getAttributeMs8rect();
                eh.hitboxType = childTag->getAttributeSimpleClass<EntityHitboxType>("type");

                frame.entityHitboxes.push_back(eh);
            }

            else if (childTag->name == "tilehitbox") {
                if (frame.solid == true) {
                    throw xml.buildError("Can only have one tilehitbox per frame");
                }
                frame.tileHitbox = childTag->getAttributeMs8rect();
                frame.solid = true;
            }

            else {
                throw childTag->buildUnknownTagError();
            }

            xml.parseCloseTag();
        }
    }

    inline void readSmallTileset(const XmlTag* tag)
    {
        assert(tag->name == "smalltileset");

        const auto data = xml.parseBase64();

        static_assert(Snes::Tile4bpp8px::SNES_DATA_SIZE == 32, "Bad assumption");
        if ((data.size() % 32) != 0) {
            throw tag->buildError("Small Tileset data must be a multiple of 32 bytes");
        }

        frameSet.smallTileset.readSnesData(data);
    }

    inline void readLargeTileset(const XmlTag* tag)
    {
        assert(tag->name == "largetileset");

        const auto data = xml.parseBase64();

        static_assert(Snes::Tile4bpp16px::SNES_DATA_SIZE == 128, "Bad assumption");
        if ((data.size() % 128) != 0) {
            throw tag->buildError("Large Tileset data must be a multiple of 128 bytes");
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
            throw tag->buildError("Palette data must contain 32 bytes");
        }

        frameSet.palettes.emplace_back(data);
    }
};

void readFrameSet(XmlReader& xml, const XmlTag* tag, FrameSet& frameSet)
{
    FrameSetReader reader(frameSet, xml);
    reader.readFrameSet(tag);
}

/*
 * FRAME SET WRITER
 * ================
 */

inline void writeFrame(XmlWriter& xml, const std::string& frameName, const Frame& frame)
{
    xml.writeTag("frame");

    xml.writeTagAttribute("id", frameName);

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
        xml.writeTagAttribute("order", obj.order);
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

        xml.writeTagAttributeSimpleClass("type", eh.hitboxType);
        xml.writeTagAttributeMs8rect(eh.aabb);

        xml.writeCloseTag();
    }

    xml.writeCloseTag();
}

inline void writeFrameSet(XmlWriter& xml, const FrameSet& frameSet)
{
    xml.writeTag("metasprite");

    xml.writeTagAttribute("id", frameSet.name);

    xml.writeTagAttributeSimpleClass("tilesettype", frameSet.tilesetType);

    // ::TODO export order document::
    // ::TODO use external module::

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

    for (const auto fIt : frameSet.frames) {
        writeFrame(xml, fIt.first, fIt.second);
    }

    Animation::writeAnimations(xml, frameSet.animations);

    xml.writeCloseTag();
}
}
}
}
