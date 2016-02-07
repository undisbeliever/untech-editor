#include "serializer.h"
#include "frameset.h"
#include "framesetgrid.h"
#include "frame.h"
#include "frameobject.h"
#include "actionpoint.h"
#include "entityhitbox.h"
#include "../common/xml.h"
#include <cassert>

// ::TODO improve error reporting::

using namespace UnTech;
using namespace UnTech::Xml;
using namespace UnTech::SpriteImporter;

namespace UnTech {
namespace SpriteImporter {
namespace Serializer {

struct FrameSetReader {
    FrameSetReader(NamedList<FrameSet>& framesetContainer, XmlReader& xml, const std::string& fileDir, const std::string& fileName)
        : framesetContainer(framesetContainer)
        , xml(xml)
        , fileDir(fileDir)
        , fileName(fileName)
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
    const std::string& fileDir;
    const std::string& fileName;

    std::shared_ptr<FrameSet> frameset;
    bool framesetGridSet;
};

void readSpriteImporter(NamedList<FrameSet>& framesetContainer, XmlReader& xml, const XmlTag* tag, const std::string& fileDir, const std::string& fileName)
{
    assert(tag->name == "spriteimporter");

    FrameSetReader reader(framesetContainer, xml, fileDir, fileName);

    std::unique_ptr<XmlTag> childTag;
    while (childTag = xml.parseTag()) {
        if (childTag->name == "frameset") {
            reader.readFrameSet(childTag.get());
        }
        else {
            throw("Unknown tag");
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
        throw("frameset id already exists");
    }

    frameset = framesetContainer.create(id);
    framesetGridSet = false;

    std::string imageFilename = tag->getAttribute("image");
    // ::TODO add dir to imageFilename::
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
            throw("Unknown tag");
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
        throw("frame id already exists");
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
                throw("Frameset grid is not set.");
            }
            frame->setGridLocation(childTag->getAttributeUpoint());
        }
        else {
            throw("location or gridlocation tag must be the first child of frame");
        }
        xml.parseCloseTag();
    }
    else {
        throw("location or gridlocation tag must be the first child of frame");
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
                throw("Unknown size");
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
                throw("Can only have one tilehitbox per frame");
            }
            frame->setTileHitbox(childTag->getAttributeUrectInside(frameLocation));
            processedTileHitbox = true;
        }

        else if (childTag->name == "origin") {
            if (processedOrigin) {
                throw("Can only have one tilehitbox per frame");
            }
            frame->setOrigin(childTag->getAttributeUpoint("x", "y"));
            processedOrigin = true;
        }
        else {
            throw("Unknown tag");
        }

        xml.parseCloseTag();
    }

    // Frame is solid only if tileHitbox exists.
    frame->setSolid(processedTileHitbox);
}
}
}
}
