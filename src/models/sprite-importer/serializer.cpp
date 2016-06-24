#include "serializer.h"
#include "actionpoint.h"
#include "entityhitbox.h"
#include "frame.h"
#include "frameobject.h"
#include "frameset.h"
#include "framesetgrid.h"
#include "models/common/atomicofstream.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"
#include "models/metasprite-common/animationserializer.h"
#include "models/metasprite-common/framesetexportorder.h"
#include <cassert>
#include <fstream>
#include <stdexcept>

using namespace UnTech;
using namespace UnTech::Xml;
using namespace UnTech::SpriteImporter;
namespace MSC = UnTech::MetaSpriteCommon;

namespace UnTech {
namespace SpriteImporter {
namespace Serializer {

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
    inline void readFrameSet(const XmlTag* tag)
    {
        assert(tag->name == "spriteimporter");
        assert(frameSet.frames().size() == 0);

        std::string id = tag->getAttributeId("id");
        frameSet.setName(id);

        frameSet.setTilesetType(
            tag->getAttributeSimpleClass<MSC::TilesetType>("tilesettype"));

        frameSetGridSet = false;

        if (tag->hasAttribute("image")) {
            std::string imageFilename = tag->getAttributeFilename("image");
            frameSet.setImageFilename(imageFilename);
        }

        if (tag->hasAttribute("transparent")) {
            static_assert(sizeof(unsigned) >= 3, "Unsigned value too small");

            UnTech::rgba color(tag->getAttributeUnsignedHex("transparent"));
            color.alpha = 0xFF;
            frameSet.setTransparentColor(color);
        }

        std::unique_ptr<XmlTag> childTag;
        while ((childTag = xml.parseTag())) {
            if (childTag->name == "grid") {
                readFrameSetGrid(childTag.get());
            }
            else if (childTag->name == "exportorder") {
                readExportOrder(childTag.get());
            }
            else if (childTag->name == "frame") {
                readFrame(childTag.get());
            }
            else if (childTag->name == "animation") {
                MSC::readAnimation(xml, childTag.get(), frameSet.animations());
            }
            else {
                throw childTag->buildUnknownTagError();
            }

            xml.parseCloseTag();
        }
    }

private:
    inline void readFrameSetGrid(const XmlTag* tag)
    {
        assert(tag->name == "grid");

        frameSet.grid().setFrameSize(tag->getAttributeUsize("width", "height"));
        frameSet.grid().setOffset(tag->getAttributeUpoint("xoffset", "yoffset"));
        frameSet.grid().setPadding(tag->getAttributeUsize("xpadding", "ypadding"));
        frameSet.grid().setOrigin(tag->getAttributeUpoint("xorigin", "yorigin"));

        frameSetGridSet = true;
    }

    inline void readExportOrder(const XmlTag* tag)
    {
        assert(tag->name == "exportorder");

        if (frameSet.exportOrderDocument() != nullptr) {
            throw tag->buildError("Only one exportorder tag allowed per frameset");
        }

        const std::string src = tag->getAttributeFilename("src");
        try {
            frameSet.loadExportOrderDocument(src);
        }
        catch (const std::exception& ex) {
            throw tag->buildError("Unable to open Export Order document", ex);
        }
    }

    inline void readFrame(const XmlTag* tag)
    {
        assert(tag->name == "frame");

        std::string id = tag->getAttributeId("id");
        if (frameSet.frames().nameExists(id)) {
            throw tag->buildError("frame id already exists");
        }

        auto framePtr = frameSet.frames().create(id);
        if (framePtr == nullptr) {
            throw std::logic_error("Unable to create frame");
        }
        Frame& frame = *framePtr;

        if (tag->hasAttribute("order")) {
            frame.setSpriteOrder(tag->getAttributeUnsigned("order", 0, 3));
        }

        std::unique_ptr<XmlTag> childTag = xml.parseTag();
        if (childTag) {
            if (childTag->name == "location") {
                auto location = childTag->getAttributeUrect(Frame::MIN_SIZE);

                frame.setUseGridLocation(false);
                frame.setLocation(location);
            }
            else if (childTag->name == "gridlocation") {
                if (frameSetGridSet == false) {
                    throw childTag->buildError("Frameset grid is not set.");
                }

                frame.setUseGridLocation(true);
                frame.setGridLocation(childTag->getAttributeUpoint());
            }
            else {
                throw tag->buildError("location or gridlocation tag must be the first child of frame");
            }
            xml.parseCloseTag();
        }
        else {
            throw tag->buildError("location or gridlocation tag must be the first child of frame");
        }

        const urect frameLocation = frame.location();
        bool processedTileHitbox = false;
        bool processedOrigin = false;

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
                    throw childTag->buildError("Unknown object size");
                }

                obj.setLocation(childTag->getAttributeUpointInside(frameLocation, obj.sizePx()));
            }

            else if (childTag->name == "actionpoint") {
                ActionPoint& ap = frame.actionPoints().create();

                ap.setLocation(childTag->getAttributeUpointInside(frameLocation));
                ap.setParameter(childTag->getAttributeUint8("parameter"));
            }

            else if (childTag->name == "entityhitbox") {
                EntityHitbox& eh = frame.entityHitboxes().create();

                eh.setAabb(childTag->getAttributeUrectInside(frameLocation));
                eh.setParameter(childTag->getAttributeUint8("parameter"));
            }

            else if (childTag->name == "tilehitbox") {
                if (processedTileHitbox) {
                    throw xml.buildError("Can only have one tilehitbox per frame");
                }
                frame.setTileHitbox(childTag->getAttributeUrectInside(frameLocation));
                processedTileHitbox = true;
            }

            else if (childTag->name == "origin") {
                if (processedOrigin) {
                    throw childTag->buildError("Can only have one tilehitbox per frame");
                }
                frame.setUseGridOrigin(false);
                frame.setOrigin(childTag->getAttributeUpoint("x", "y"));
                processedOrigin = true;
            }
            else {
                throw childTag->buildUnknownTagError();
            }

            xml.parseCloseTag();
        }

        // Frame is solid only if tileHitbox exists.
        frame.setSolid(processedTileHitbox);
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
    xml.writeTagAttribute("order", frame.spriteOrder());

    if (frame.useGridLocation()) {
        xml.writeTag("gridlocation");

        xml.writeTagAttributeUpoint(frame.gridLocation());

        xml.writeCloseTag();
    }
    else {
        xml.writeTag("location");

        xml.writeTagAttributeUrect(frame.location());

        xml.writeCloseTag();
    }

    if (frame.useGridOrigin() == false) {
        xml.writeTag("origin");

        xml.writeTagAttributeUpoint(frame.origin());

        xml.writeCloseTag();
    }

    if (frame.solid()) {
        xml.writeTag("tilehitbox");

        xml.writeTagAttributeUrect(frame.tileHitbox());

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
        xml.writeTagAttributeUpoint(obj.location());

        xml.writeCloseTag();
    }

    for (const ActionPoint& ap : frame.actionPoints()) {
        xml.writeTag("actionpoint");

        xml.writeTagAttribute("parameter", ap.parameter());
        xml.writeTagAttributeUpoint(ap.location());

        xml.writeCloseTag();
    }

    for (const EntityHitbox& eh : frame.entityHitboxes()) {
        xml.writeTag("entityhitbox");

        xml.writeTagAttribute("parameter", eh.parameter());
        xml.writeTagAttributeUrect(eh.aabb());

        xml.writeCloseTag();
    }

    xml.writeCloseTag();
}

inline void writeFrameSetGrid(XmlWriter& xml, const FrameSetGrid& grid)
{
    xml.writeTag("grid");

    xml.writeTagAttributeUsize(grid.frameSize(), "width", "height");
    xml.writeTagAttributeUpoint(grid.offset(), "xoffset", "yoffset");
    xml.writeTagAttributeUsize(grid.padding(), "xpadding", "ypadding");
    xml.writeTagAttributeUpoint(grid.origin(), "xorigin", "yorigin");

    xml.writeCloseTag();
}

inline void writeFrameSet(XmlWriter& xml, const FrameSet& frameSet)
{
    xml.writeTag("spriteimporter");

    xml.writeTagAttribute("id", frameSet.name());

    xml.writeTagAttributeSimpleClass("tilesettype", frameSet.tilesetType());

    if (!frameSet.imageFilename().empty()) {
        xml.writeTagAttributeFilename("image", frameSet.imageFilename());
    }

    if (frameSet.transparentColorValid()) {
        static_assert(sizeof(unsigned) >= 3, "Unsigned value too small");

        unsigned rgb = frameSet.transparentColor().rgb();
        xml.writeTagAttributeHex("transparent", rgb, 6);
    }

    writeFrameSetGrid(xml, frameSet.grid());

    if (frameSet.exportOrderDocument() != nullptr) {
        const std::string& src = frameSet.exportOrderDocument()->filename();

        if (!src.empty()) {
            xml.writeTag("exportorder");
            xml.writeTagAttributeFilename("src", src);
            xml.writeCloseTag();
        }
    }

    for (const auto f : frameSet.frames()) {
        writeFrame(xml, f.first, f.second);
    }

    MSC::writeAnimations(xml, frameSet.animations());

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

    if (tag->name != "spriteimporter") {
        throw std::runtime_error(filename + ": Not a sprite importer file");
    }

    FrameSetReader reader(frameSet, *xml);

    reader.readFrameSet(tag.get());
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
