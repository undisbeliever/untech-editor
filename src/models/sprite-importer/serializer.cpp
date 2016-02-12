#include "serializer.h"
#include "frameset.h"
#include "framesetgrid.h"
#include "frame.h"
#include "frameobject.h"
#include "actionpoint.h"
#include "entityhitbox.h"
#include "../common/xml/xmlreader.h"
#include <cassert>
#include <stdexcept>

using namespace UnTech;
using namespace UnTech::Xml;
using namespace UnTech::SpriteImporter;

namespace UnTech {
namespace SpriteImporter {
namespace Serializer {

struct FrameSetReader {
    FrameSetReader(NamedList<FrameSet>& framesetContainer, XmlReader& xml)
        : framesetContainer(framesetContainer)
        , xml(xml)
        , frameset()
        , framesetGridSet(false)
    {
    }

    void readFrameSet(const XmlTag* tag);

private:
    void readFrameSetGrid(const XmlTag* tag);
    void readFrame(const XmlTag* tag);

    NamedList<FrameSet>& framesetContainer;
    XmlReader& xml;

    std::shared_ptr<FrameSet> frameset;
    bool framesetGridSet;
};

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
    while (childTag = xml.parseTag()) {
        if (childTag->name == "frameset") {
            reader.readFrameSet(childTag.get());
        }
        else {
            throw tag->buildUnknownTagError();
        }

        xml.parseCloseTag();
    }
}

/*
 * FRAME SET READER
 * ================
 */

void FrameSetReader::readFrameSet(const XmlTag* tag)
{
    assert(tag->name == "frameset");

    std::string id = tag->getAttributeId("id");
    if (framesetContainer.nameExists(id)) {
        throw tag->buildError("frameset id already exists");
    }

    frameset = framesetContainer.create(id);
    framesetGridSet = false;

    std::string imageFilename = xml.dirname() + tag->getAttribute("image");
    frameset->setImageFilename(imageFilename);
    // ::TODO verify image was loaded correctly::

    std::unique_ptr<XmlTag> childTag;
    while (childTag = xml.parseTag()) {
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

void FrameSetReader::readFrameSetGrid(const XmlTag* tag)
{
    assert(tag->name == "grid");

    frameset->grid().setFrameSize(tag->getAttributeUsize("width", "height"));
    frameset->grid().setOffset(tag->getAttributeUpoint("xoffset", "yoffset"));
    frameset->grid().setPadding(tag->getAttributeUsize("xpadding", "ypadding"));
    frameset->grid().setOrigin(tag->getAttributeUpoint("xorigin", "yorigin"));

    framesetGridSet = true;
}

void FrameSetReader::readFrame(const XmlTag* tag)
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

    while (childTag = xml.parseTag()) {
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
}
}
}
