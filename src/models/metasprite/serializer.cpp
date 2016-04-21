#include "serializer.h"
#include "frameset.h"
#include "frame.h"
#include "actionpoint.h"
#include "entityhitbox.h"
#include "frameobject.h"
#include "palette.h"
#include "../common/atomicofstream.h"
#include "../common/xml/xmlreader.h"
#include "../common/xml/xmlwriter.h"
#include "../snes/palette.hpp"
#include "../snes/tileset.hpp"
#include <cassert>
#include <stdexcept>
#include <fstream>

using namespace UnTech;
using namespace UnTech::Xml;
using namespace UnTech::MetaSprite;

namespace UnTech {
namespace MetaSprite {
namespace Serializer {

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
        assert(frameSet.frames().size() == 0);

        std::string id = tag->getAttributeId("id");
        frameSet.setName(id);

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

        std::string id = tag->getAttributeId("id");
        if (frameSet.frames().nameExists(id)) {
            throw tag->buildError("frame id already exists");
        }

        Frame* framePtr = frameSet.frames().create(id);
        if (framePtr == nullptr) {
            throw std::logic_error("Could not create Frame");
        }
        Frame& frame = *framePtr;

        std::unique_ptr<XmlTag> childTag;

        bool processedTileHitbox = false;

        while ((childTag = xml.parseTag())) {
            if (childTag->name == "object") {
                FrameObject& obj = frame.objects().create();

                std::string sizeStr = childTag->getAttribute("size");

                if (sizeStr == "small") {
                    obj.setSize(FrameObject::ObjectSize::SMALL);
                }
                else if (sizeStr == "large") {
                    obj.setSize(FrameObject::ObjectSize::LARGE);
                }
                else {
                    throw childTag->buildError("size", "Unknown size");
                }

                obj.setLocation(childTag->getAttributeMs8point());
                obj.setTileId(childTag->getAttributeUnsigned("tile"));
                obj.setOrder(childTag->getAttributeUnsigned("order", 0, FrameObject::ORDER_MASK));
                obj.setHFlip(childTag->getAttributeBoolean("hflip"));
                obj.setVFlip(childTag->getAttributeBoolean("vflip"));
            }

            else if (childTag->name == "actionpoint") {
                ActionPoint& ap = frame.actionPoints().create();

                ap.setLocation(childTag->getAttributeMs8point());
                ap.setParameter(childTag->getAttributeUint8("parameter"));
            }

            else if (childTag->name == "entityhitbox") {
                EntityHitbox& eh = frame.entityHitboxes().create();

                eh.setAabb(childTag->getAttributeMs8rect());
                eh.setParameter(childTag->getAttributeUint8("parameter"));
            }

            else if (childTag->name == "tilehitbox") {
                if (processedTileHitbox) {
                    throw xml.buildError("Can only have one tilehitbox per frame");
                }
                frame.setTileHitbox(childTag->getAttributeMs8rect());
                processedTileHitbox = true;
            }

            else {
                throw childTag->buildUnknownTagError();
            }

            xml.parseCloseTag();
        }

        // Frame is solid only if tileHitbox exists.
        frame.setSolid(processedTileHitbox);
    }

    inline void readSmallTileset(const XmlTag* tag)
    {
        assert(tag->name == "smalltileset");

        const auto data = xml.parseBase64();

        static_assert(Snes::Tileset4bpp8px::SNES_DATA_SIZE == 32, "Bad assumption");
        if ((data.size() % 32) != 0) {
            throw tag->buildError("Small Tileset data must be a multiple of 32 bytes");
        }

        frameSet.smallTileset().readSnesData(data);
    }

    inline void readLargeTileset(const XmlTag* tag)
    {
        assert(tag->name == "largetileset");

        const auto data = xml.parseBase64();

        static_assert(Snes::Tileset4bpp16px::SNES_DATA_SIZE == 128, "Bad assumption");
        if ((data.size() % 128) != 0) {
            throw tag->buildError("Large Tileset data must be a multiple of 128 bytes");
        }

        frameSet.largeTileset().readSnesData(data);
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

        Palette& palette = frameSet.palettes().create();
        palette.readPalette(data);
    }
};

/*
 * FRAME SET WRITER
 * ================
 */

namespace FrameSetWriter {

inline void writeFrame(XmlWriter& xml, const std::string& frameName, const Frame& frame)
{
    xml.writeTag("frame");

    xml.writeTagAttribute("id", frameName);

    if (frame.solid()) {
        xml.writeTag("tilehitbox");

        xml.writeTagAttributeMs8rect(frame.tileHitbox());

        xml.writeCloseTag();
    }

    for (const FrameObject& obj : frame.objects()) {
        xml.writeTag("object");

        if (obj.size() == FrameObject::ObjectSize::SMALL) {
            xml.writeTagAttribute("size", "small");
        }
        else {
            xml.writeTagAttribute("size", "large");
        }

        xml.writeTagAttributeMs8point(obj.location());
        xml.writeTagAttribute("tile", obj.tileId());
        xml.writeTagAttribute("order", obj.order());
        xml.writeTagAttribute("hflip", obj.hFlip());
        xml.writeTagAttribute("vflip", obj.vFlip());

        xml.writeCloseTag();
    }

    for (const ActionPoint& ap : frame.actionPoints()) {
        xml.writeTag("actionpoint");

        xml.writeTagAttribute("parameter", ap.parameter());
        xml.writeTagAttributeMs8point(ap.location());

        xml.writeCloseTag();
    }

    for (const EntityHitbox& eh : frame.entityHitboxes()) {
        xml.writeTag("entityhitbox");

        xml.writeTagAttribute("parameter", eh.parameter());
        xml.writeTagAttributeMs8rect(eh.aabb());

        xml.writeCloseTag();
    }

    xml.writeCloseTag();
}

inline void writeFrameSet(XmlWriter& xml, const FrameSet& frameSet)
{
    xml.writeTag("metasprite");

    xml.writeTagAttribute("id", frameSet.name());

    if (frameSet.smallTileset().size()) {
        xml.writeTag("smalltileset");
        xml.writeBase64(frameSet.smallTileset().snesData());
        xml.writeCloseTag();
    }

    if (frameSet.largeTileset().size()) {
        xml.writeTag("largetileset");
        xml.writeBase64(frameSet.largeTileset().snesData());
        xml.writeCloseTag();
    }

    for (const Palette& p : frameSet.palettes()) {
        xml.writeTag("palette");
        xml.writeBase64(p.paletteData());
        xml.writeCloseTag();
    }

    for (const auto fIt : frameSet.frames()) {
        writeFrame(xml, fIt.first, fIt.second);
    }

    xml.writeCloseTag();
}
}

/*
 * API
 * ===
 */

void readFile(FrameSet& frameSet, const std::string& filename)
{
    auto xml = XmlReader::fromFile(filename);
    std::unique_ptr<XmlTag> tag = xml->parseTag();

    if (tag->name != "metasprite") {
        throw std::runtime_error(filename + ": Not a meta sprite file");
    }

    FrameSetReader reader(frameSet, *xml);

    reader.readFrameSet(tag.get());
}

// ::TODO remove when completed utsi2utms command line argument parsing"
void writeFile(const FrameSet& frameSet, std::ostream& file)
{
    XmlWriter xml(file, "untech");

    FrameSetWriter::writeFrameSet(xml, frameSet);
}

void writeFile(const FrameSet& frameSet, const std::string& filename)
{
    UnTech::AtomicOfStream file(filename);

    XmlWriter xml(file, filename, "untech");

    FrameSetWriter::writeFrameSet(xml, frameSet);

    file.commit();
}
}
}
}
