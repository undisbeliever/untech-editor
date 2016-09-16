#include "frameset-exportorder.h"

#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"
#include <cassert>
#include <stdexcept>

using namespace UnTech;
using namespace UnTech::Xml;

namespace UnTech {
namespace MetaSprite {

const std::string FrameSetExportOrder::FILE_EXTENSION = "utfseo";

template <>
const std::string FrameSetExportOrder::ExportName::list_t::HUMAN_TYPE_NAME = "Export Name";

struct FrameSetExportOrderReader {
    FrameSetExportOrderReader(FrameSetExportOrder& exportOrder, XmlReader& xml)
        : exportOrder(exportOrder)
        , xml(xml)
    {
    }

private:
    FrameSetExportOrder& exportOrder;
    XmlReader& xml;

public:
    inline void readFrameSetExportOrder(const XmlTag* tag)
    {
        // ::TODO change root tag::
        assert(tag->name == "framesettype");
        assert(exportOrder.stillFrames.size() == 0);
        assert(exportOrder.animations.size() == 0);

        exportOrder.name = tag->getAttributeId("name");

        std::unique_ptr<XmlTag> childTag;

        while ((childTag = xml.parseTag())) {
            if (childTag->name == "frame") {
                auto en = readExportName(childTag.get());
                // ::TODO detect duplicate names::
                exportOrder.stillFrames.push_back(en);
            }
            else if (childTag->name == "animation") {
                auto en = readExportName(childTag.get());
                // ::TODO detect duplicate names::
                exportOrder.animations.push_back(en);
            }
            else {
                throw childTag->buildUnknownTagError();
            }

            xml.parseCloseTag();
        }
    }

private:
    inline FrameSetExportOrder::ExportName readExportName(const XmlTag* tag)
    {
        FrameSetExportOrder::ExportName en;
        en.name = tag->getAttributeId("id");

        std::unique_ptr<XmlTag> childTag;

        while ((childTag = xml.parseTag())) {
            if (childTag->name == "alt") {
                NameReference alt;

                alt.name = childTag->getAttributeId("name");
                alt.hFlip = childTag->getAttributeBoolean("hflip");
                alt.vFlip = childTag->getAttributeBoolean("vflip");

                en.alternatives.push_back(alt);
            }
            else {
                throw childTag->buildUnknownTagError();
            }

            xml.parseCloseTag();
        }

        return en;
    }
};

// ::SHOULDO cache these values for metasprite-compiler::
std::shared_ptr<const FrameSetExportOrder>
loadFrameSetExportOrder(const std::string& filename)
{
    auto xml = XmlReader::fromFile(filename);
    std::unique_ptr<XmlTag> tag = xml->parseTag();

    // ::TODO change root tag::
    if (tag->name != "framesettype") {
        throw std::runtime_error(filename + ": Not frame set export order file");
    }

    auto exportOrder = std::make_shared<FrameSetExportOrder>();

    FrameSetExportOrderReader reader(*exportOrder, *xml);
    reader.readFrameSetExportOrder(tag.get());

    exportOrder->filename = File::fullPath(filename);

    return std::const_pointer_cast<const FrameSetExportOrder>(exportOrder);
}
}
}
