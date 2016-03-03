#include "serializer.h"
#include "frameset.h"
#include "framesetgrid.h"
#include "frame.h"
#include "frameobject.h"
#include "actionpoint.h"
#include "entityhitbox.h"
#include "../common/xml/xmlreader.h"
#include "../common/xml/xmlwriter.h"
#include <cassert>
#include <stdexcept>
#include <fstream>

using namespace UnTech;
using namespace UnTech::Xml;
using namespace UnTech::SpriteImporter;

namespace UnTech {
namespace SpriteImporter {
namespace Serializer {

/*
 * FRAME SET READER
 * ================
 */

struct FrameSetReader {
    FrameSetReader(NamedList<FrameSet>& framesetContainer, XmlReader& xml)
        : framesetContainer(framesetContainer)
        , xml(xml)
        , frameset()
        , framesetGridSet(false)
    {
    }

private:
    NamedList<FrameSet>& framesetContainer;
    XmlReader& xml;

    std::shared_ptr<FrameSet> frameset;
    bool framesetGridSet;

public:
    inline void readFrameSet(const XmlTag* tag)
    {
        assert(tag->name == "frameset");

        std::string id = tag->getAttributeId("id");
        if (framesetContainer.nameExists(id)) {
            throw tag->buildError("frameset id already exists");
        }

        frameset = framesetContainer.create(id);
        framesetGridSet = false;

        std::string imageAttribute = tag->getAttribute("image");
        std::string imageFilename = xml.dirname() + imageAttribute;
        frameset->setImageFilename(imageFilename);

        if (tag->hasAttribute("transparent")) {
            static_assert(sizeof(unsigned) >= 3, "Unsigned value too small");

            UnTech::rgba color(tag->getAttributeUnsignedHex("transparent"));
            color.alpha = 0xFF;
            frameset->setTransparentColor(color);
        }

        std::unique_ptr<XmlTag> childTag;
        while ((childTag = xml.parseTag())) {
            if (childTag->name == "grid") {
                readFrameSetGrid(childTag.get());
            }
            else if (childTag->name == "frame") {
                readFrame(childTag.get());
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

        frameset->grid().setFrameSize(tag->getAttributeUsize("width", "height"));
        frameset->grid().setOffset(tag->getAttributeUpoint("xoffset", "yoffset"));
        frameset->grid().setPadding(tag->getAttributeUsize("xpadding", "ypadding"));
        frameset->grid().setOrigin(tag->getAttributeUpoint("xorigin", "yorigin"));

        framesetGridSet = true;
    }

    inline void readFrame(const XmlTag* tag)
    {
        assert(tag->name == "frame");

        std::string id = tag->getAttributeId("id");
        if (frameset->frames().nameExists(id)) {
            throw tag->buildError("frame id already exists");
        }

        auto frame = frameset->frames().create(id);

        if (tag->hasAttribute("order")) {
            frame->setSpriteOrder(tag->getAttributeUnsigned("order", 0, 3));
        }

        std::unique_ptr<XmlTag> childTag = xml.parseTag();
        if (childTag) {
            if (childTag->name == "location") {
                auto location = childTag->getAttributeUrect(Frame::MIN_SIZE);

                frame->setLocation(location);
            }
            if (childTag->name == "gridlocation") {
                if (framesetGridSet == false) {
                    throw childTag->buildError("Frameset grid is not set.");
                }
                frame->setGridLocation(childTag->getAttributeUpoint());
            }
            else {
                throw tag->buildError("location or gridlocation tag must be the first child of frame");
            }
            xml.parseCloseTag();
        }
        else {
            throw tag->buildError("location or gridlocation tag must be the first child of frame");
        }

        const urect frameLocation = frame->location();
        bool processedTileHitbox = false;
        bool processedOrigin = false;

        while ((childTag = xml.parseTag())) {
            if (childTag->name == "object") {
                auto obj = frame->objects().create();

                std::string sizeStr = childTag->getAttribute("size");

                if (sizeStr == "small") {
                    obj->setSize(FrameObject::ObjectSize::SMALL);
                }
                else if (sizeStr == "large") {
                    obj->setSize(FrameObject::ObjectSize::LARGE);
                }
                else {
                    throw childTag->buildError("Can only have one tilehitbox per frame");
                }

                obj->setLocation(childTag->getAttributeUpointInside(frameLocation, obj->sizePx()));
            }

            else if (childTag->name == "actionpoint") {
                auto ap = frame->actionPoints().create();

                ap->setLocation(childTag->getAttributeUpointInside(frameLocation));
                ap->setParameter(childTag->getAttributeUint8("parameter"));
            }

            else if (childTag->name == "entityhitbox") {
                auto eh = frame->entityHitboxes().create();

                eh->setAabb(childTag->getAttributeUrectInside(frameLocation));
                eh->setParameter(childTag->getAttributeUint8("parameter"));
            }

            else if (childTag->name == "tilehitbox") {
                if (processedTileHitbox) {
                    throw xml.buildError("Can only have one tilehitbox per frame");
                }
                frame->setTileHitbox(childTag->getAttributeUrectInside(frameLocation));
                processedTileHitbox = true;
            }

            else if (childTag->name == "origin") {
                if (processedOrigin) {
                    throw childTag->buildError("Can only have one tilehitbox per frame");
                }
                frame->setOrigin(childTag->getAttributeUpoint("x", "y"));
                processedOrigin = true;
            }
            else {
                throw childTag->buildUnknownTagError();
            }

            xml.parseCloseTag();
        }

        // Frame is solid only if tileHitbox exists.
        frame->setSolid(processedTileHitbox);
    }
};

/*
 * FRAME SET WRITER
 * ================
 */

namespace FrameSetWriter {

inline void writeFrame(XmlWriter& xml, const std::string& frameName, const Frame* frame)
{
    xml.writeTag("frame");

    xml.writeTagAttribute("id", frameName);
    xml.writeTagAttribute("order", frame->spriteOrder());

    if (frame->useGridLocation()) {
        xml.writeTag("gridlocation");

        xml.writeTagAttributeUpoint(frame->gridLocation());

        xml.writeCloseTag();
    }
    else {
        xml.writeTag("location");

        xml.writeTagAttributeUrect(frame->location());

        xml.writeCloseTag();
    }

    if (frame->useGridOrigin() == false) {
        xml.writeTag("origin");

        xml.writeTagAttributeUpoint(frame->origin());

        xml.writeCloseTag();
    }

    if (frame->solid()) {
        xml.writeTag("tilehitbox");

        xml.writeTagAttributeUrect(frame->tileHitbox());

        xml.writeCloseTag();
    }

    for (const auto obj : frame->objects()) {
        xml.writeTag("object");

        if (obj->size() == FrameObject::ObjectSize::SMALL) {
            xml.writeTagAttribute("size", "small");
        }
        else {
            xml.writeTagAttribute("size", "large");
        }
        xml.writeTagAttributeUpoint(obj->location());

        xml.writeCloseTag();
    }

    for (const auto ap : frame->actionPoints()) {
        xml.writeTag("actionpoint");

        xml.writeTagAttribute("parameter", ap->parameter());
        xml.writeTagAttributeUpoint(ap->location());

        xml.writeCloseTag();
    }

    for (const auto eh : frame->entityHitboxes()) {
        xml.writeTag("entityhitbox");

        xml.writeTagAttribute("parameter", eh->parameter());
        xml.writeTagAttributeUrect(eh->aabb());

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

inline void writeFrameSet(XmlWriter& xml, const std::string& framesetName, const FrameSet* frameset)
{
    xml.writeTag("frameset");

    xml.writeTagAttribute("id", framesetName);

    // ::TODO get relative image filename::
    std::string imageFilename = frameset->imageFilename();
    xml.writeTagAttribute("image", imageFilename);

    if (frameset->transparentColorValid()) {
        static_assert(sizeof(unsigned) >= 3, "Unsigned value too small");

        unsigned rgb = frameset->transparentColor().rgb();
        xml.writeTagAttributeHex("transparent", rgb, 6);
    }

    writeFrameSetGrid(xml, frameset->grid());

    for (const auto& f : frameset->frames()) {
        writeFrame(xml, f.first, f.second.get());
    }

    xml.writeCloseTag();
}

inline void writeFrameSetList(XmlWriter& xml, const NamedList<FrameSet>& framesetContainer)
{
    xml.writeTag("spriteimporter");

    for (const auto& fs : framesetContainer) {
        writeFrameSet(xml, fs.first, fs.second.get());
    }

    xml.writeCloseTag();
}
}

/*
 * API
 * ===
 */

void readFile(NamedList<FrameSet>& frameSetContainer, const std::string& filename)
{
    auto xml = XmlReader::fromFile(filename);
    std::unique_ptr<XmlTag> tag = xml->parseTag();

    if (tag->name != "spriteimporter") {
        throw std::runtime_error(filename + ": Not a sprite importer file");
    }

    readSpriteImporter(frameSetContainer, *xml, tag.get());
}

void readSpriteImporter(NamedList<FrameSet>& framesetContainer, XmlReader& xml, const XmlTag* tag)
{
    assert(tag->name == "spriteimporter");

    FrameSetReader reader(framesetContainer, xml);

    std::unique_ptr<XmlTag> childTag;
    while ((childTag = xml.parseTag())) {
        if (childTag->name == "frameset") {
            reader.readFrameSet(childTag.get());
        }
        else {
            throw tag->buildUnknownTagError();
        }

        xml.parseCloseTag();
    }
}

void writeFile(const NamedList<FrameSet>& frameSetContainer, std::ostream& file)
{
    XmlWriter xml(file, "untech");

    FrameSetWriter::writeFrameSetList(xml, frameSetContainer);
}

void writeFile(const NamedList<FrameSet>& frameSetContainer, const std::string& filename)
{
    // ::TODO implement Qt SaveFile class for atomicity::

    std::ofstream file(filename, std::ios_base::out);
    writeFile(frameSetContainer, file);
}

void writeSpriteImporter(const NamedList<FrameSet>& frameSetContainer, XmlWriter& xml)
{
    FrameSetWriter::writeFrameSetList(xml, frameSetContainer);
}
}
}
}
