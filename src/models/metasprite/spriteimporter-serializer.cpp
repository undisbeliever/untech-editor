#include "spriteimporter-serializer.h"
#include "animation/serializer.h"

#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"
#include <cassert>
#include <fstream>
#include <stdexcept>

using namespace UnTech;
using namespace UnTech::Xml;

namespace UnTech {
namespace MetaSprite {
namespace SpriteImporter {

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
        assert(frameSet.frames.size() == 0);

        frameSet.name = tag->getAttributeId("id");

        frameSet.tilesetType = tag->getAttributeSimpleClass<TilesetType>("tilesettype");

        frameSetGridSet = false;

        if (tag->hasAttribute("image")) {
            std::string imageFilename = tag->getAttributeFilename("image");
            frameSet.loadImage(imageFilename);
        }

        if (tag->hasAttribute("transparent")) {
            static_assert(sizeof(unsigned) >= 3, "Unsigned value too small");

            unsigned hex = tag->getAttributeUnsignedHex("transparent");

            frameSet.transparentColor = UnTech::rgba::fromRgba(hex);
            frameSet.transparentColor.alpha = 0xFF;
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
                Animation::readAnimation(xml, childTag.get(), frameSet.animations);
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

        frameSet.grid.frameSize = tag->getAttributeUsize("width", "height");
        frameSet.grid.offset = tag->getAttributeUpoint("xoffset", "yoffset");
        frameSet.grid.padding = tag->getAttributeUsize("xpadding", "ypadding");
        frameSet.grid.origin = tag->getAttributeUpoint("xorigin", "yorigin");

        frameSetGridSet = true;
    }

    inline void readFrame(const XmlTag* tag)
    {
        assert(tag->name == "frame");

        std::string id = tag->getAttributeUniqueId("id", frameSet.frames);
        Frame& frame = frameSet.frames[id];

        if (tag->hasAttribute("order")) {
            frame.spriteOrder = tag->getAttributeUnsigned("order", 0, frame.spriteOrder.MASK);
        }

        std::unique_ptr<XmlTag> childTag = xml.parseTag();
        if (childTag) {
            if (childTag->name == "location") {
                static const usize minSize(MIN_FRAME_SIZE, MIN_FRAME_SIZE);

                frame.location.aabb = childTag->getAttributeUrect(minSize);
                frame.location.useGridLocation = false;
            }
            else if (childTag->name == "gridlocation") {
                if (frameSetGridSet == false) {
                    throw childTag->buildError("Frameset grid is not set.");
                }

                frame.location.gridLocation = childTag->getAttributeUpoint();
                frame.location.useGridLocation = true;
            }
            else {
                throw tag->buildError("location or gridlocation tag must be the first child of frame");
            }
            xml.parseCloseTag();
        }
        else {
            throw tag->buildError("location or gridlocation tag must be the first child of frame");
        }

        frame.location.useGridOrigin = true;
        frame.location.update(frameSet.grid, frame);
        const urect frameLocation = frame.location.aabb;

        frame.solid = false;

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
                    throw childTag->buildError("Unknown object size");
                }

                obj.location = childTag->getAttributeUpointInside(frameLocation, obj.sizePx());

                frame.objects.push_back(obj);
            }

            else if (childTag->name == "actionpoint") {
                ActionPoint ap;

                ap.location = childTag->getAttributeUpointInside(frameLocation);
                ap.parameter = childTag->getAttributeClamped<ActionPointParameter>("parameter");

                frame.actionPoints.push_back(ap);
            }

            else if (childTag->name == "entityhitbox") {
                EntityHitbox eh;

                eh.aabb = childTag->getAttributeUrectInside(frameLocation);
                eh.hitboxType = childTag->getAttributeSimpleClass<EntityHitboxType>("type");

                frame.entityHitboxes.push_back(eh);
            }

            else if (childTag->name == "tilehitbox") {
                if (frame.solid) {
                    throw xml.buildError("Can only have one tilehitbox per frame");
                }
                frame.tileHitbox = childTag->getAttributeUrectInside(frameLocation);
                frame.solid = true;
            }

            else if (childTag->name == "origin") {
                if (frame.location.useGridOrigin == false) {
                    throw childTag->buildError("Can only have one origin per frame");
                }
                frame.location.useGridOrigin = false;
                frame.location.origin = childTag->getAttributeUpointInside(frameLocation);
            }
            else {
                throw childTag->buildUnknownTagError();
            }

            xml.parseCloseTag();
        }

        // ::TODO frame.isValid()::
    }

    inline void readExportOrder(const XmlTag* tag)
    {
        assert(tag->name == "exportorder");

        if (frameSet.exportOrder != nullptr) {
            throw tag->buildError("Only one exportorder tag allowed per frameset");
        }

        const std::string src = tag->getAttributeFilename("src");
        try {
            frameSet.exportOrder = loadFrameSetExportOrder(src);
        }
        catch (const std::exception& ex) {
            throw tag->buildError("Unable to open FrameSet Export Order", ex);
        }
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

    if (frame.solid) {
        xml.writeTag("tilehitbox");
        xml.writeTagAttributeUrect(frame.tileHitbox);
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
        xml.writeTagAttributeUpoint(obj.location);

        xml.writeCloseTag();
    }

    for (const ActionPoint& ap : frame.actionPoints) {
        xml.writeTag("actionpoint");

        xml.writeTagAttribute("parameter", ap.parameter);
        xml.writeTagAttributeUpoint(ap.location);

        xml.writeCloseTag();
    }

    for (const EntityHitbox& eh : frame.entityHitboxes) {
        xml.writeTag("entityhitbox");

        xml.writeTagAttributeSimpleClass("type", eh.hitboxType);
        xml.writeTagAttributeUrect(eh.aabb);

        xml.writeCloseTag();
    }

    xml.writeCloseTag();
}

inline void writeFrameSetGrid(XmlWriter& xml, const FrameSetGrid& grid)
{
    xml.writeTag("grid");

    xml.writeTagAttributeUsize(grid.frameSize, "width", "height");
    xml.writeTagAttributeUpoint(grid.offset, "xoffset", "yoffset");
    xml.writeTagAttributeUsize(grid.padding, "xpadding", "ypadding");
    xml.writeTagAttributeUpoint(grid.origin, "xorigin", "yorigin");

    xml.writeCloseTag();
}

inline void writeFrameSet(XmlWriter& xml, const FrameSet& frameSet)
{
    xml.writeTag("spriteimporter");

    xml.writeTagAttribute("id", frameSet.name);

    xml.writeTagAttributeSimpleClass("tilesettype", frameSet.tilesetType);

    if (!frameSet.imageFilename.empty()) {
        xml.writeTagAttributeFilename("image", frameSet.imageFilename);
    }

    if (frameSet.transparentColorValid()) {
        static_assert(sizeof(unsigned) >= 3, "Unsigned value too small");

        unsigned rgb = frameSet.transparentColor.rgb();
        xml.writeTagAttributeHex("transparent", rgb, 6);
    }

    writeFrameSetGrid(xml, frameSet.grid);

    if (frameSet.exportOrder != nullptr) {
        const std::string& src = frameSet.exportOrder->filename;

        if (!src.empty()) {
            xml.writeTag("exportorder");
            xml.writeTagAttributeFilename("src", src);
            xml.writeCloseTag();
        }
    }

    for (const auto f : frameSet.frames) {
        writeFrame(xml, f.first, f.second);
    }

    Animation::writeAnimations(xml, frameSet.animations);

    xml.writeCloseTag();
}
}
}
}
