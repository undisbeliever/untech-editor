#include "frameset-exportorder.h"

#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"
#include <cassert>
#include <stdexcept>

using namespace UnTech;
using namespace UnTech::Xml;

namespace MS = UnTech::MetaSprite;

const std::string MS::FrameSetExportOrder::FILE_EXTENSION = "utfseo";
template <>
const std::string MS::FrameSetExportOrder::ExportName::list_t::HUMAN_TYPE_NAME = "Export Name";

namespace UnTech {
namespace MetaSprite {

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
                throw unknown_tag_error(*childTag);
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
                throw unknown_tag_error(*childTag);
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
    try {
        std::unique_ptr<XmlTag> tag = xml->parseTag();

        // ::TODO change root tag::
        if (tag->name != "framesettype") {
            throw std::runtime_error(filename + ": Not frame set export order file");
        }

        auto exportOrder = std::make_shared<FrameSetExportOrder>();
        exportOrder->filename = File::fullPath(filename);

        FrameSetExportOrderReader reader(*exportOrder, *xml);
        reader.readFrameSetExportOrder(tag.get());

        return std::const_pointer_cast<const FrameSetExportOrder>(exportOrder);
    }
    catch (const std::exception& ex) {
        throw xml_error(*xml, "Error loading FrameSetExportOrder file", ex);
    }
}
}
}
