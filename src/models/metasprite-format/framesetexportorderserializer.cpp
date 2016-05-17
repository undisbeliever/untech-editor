#include "framesetexportorderserializer.h"
#include "framesetexportorder.h"
#include "models/common/atomicofstream.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"
#include <cassert>
#include <fstream>
#include <stdexcept>

using namespace UnTech;
using namespace UnTech::Xml;
using namespace UnTech::MetaSpriteFormat;

namespace UnTech {
namespace MetaSpriteFormat {
namespace FrameSetExportOrder {
namespace Serializer {

/*
 * EXPORT ORDER READER
 * ===================
 */

struct ExportOrderReader {
    ExportOrderReader(ExportOrder& exportOrder, XmlReader& xml)
        : exportOrder(exportOrder)
        , xml(xml)
    {
    }

private:
    ExportOrder& exportOrder;
    XmlReader& xml;

public:
    inline void readExportOrder(const XmlTag* tag)
    {
        assert(tag->name == "framesettype");
        assert(exportOrder.stillFrames().size() == 0);
        assert(exportOrder.animations().size() == 0);

        std::string name = tag->getAttributeId("name");

        exportOrder.setName(name);

        std::unique_ptr<XmlTag> childTag;

        while ((childTag = xml.parseTag())) {
            if (childTag->name == "frame") {
                readExportName(exportOrder.stillFrames(), childTag.get());
            }
            else if (childTag->name == "animation") {
                readExportName(exportOrder.animations(), childTag.get());
            }
            else {
                throw childTag->buildUnknownTagError();
            }

            xml.parseCloseTag();
        }
    }

private:
    inline void readExportName(ExportName::list_t& list, const XmlTag* tag)
    {
        std::string id = tag->getAttributeId("id");
        if (list.nameExists(id)) {
            throw tag->buildError("id already exists");
        }

        ExportName* ns = list.create(id);
        if (ns == nullptr) {
            throw std::logic_error("Could not create ExportName");
        }

        auto& alternativeNames = ns->alternativeNames();

        std::unique_ptr<XmlTag> childTag;

        while ((childTag = xml.parseTag())) {
            if (childTag->name == "alt") {
                AlternativeName& alt = alternativeNames.create();

                alt.setName(childTag->getAttributeId("name"));
                alt.setHFlip(childTag->getAttributeBoolean("hflip"));
                alt.setVFlip(childTag->getAttributeBoolean("vflip"));
            }
            else {
                throw childTag->buildUnknownTagError();
            }

            xml.parseCloseTag();
        }
    }
};

/*
 * EXPORT ORDER WRITER
 * ===================
 */

namespace ExportOrderWriter {

inline void writeExportName(XmlWriter& xml, const std::string& type,
                            const std::string& name, const ExportName& exportName)
{
    xml.writeTag(type);

    xml.writeTagAttribute("id", name);

    for (const AlternativeName& alt : exportName.alternativeNames()) {
        xml.writeTag("alt");

        xml.writeTagAttribute("name", alt.name());
        xml.writeTagAttribute("hflip", alt.hFlip());
        xml.writeTagAttribute("vflip", alt.vFlip());

        xml.writeCloseTag();
    }

    xml.writeCloseTag();
}

inline void writeExportOrder(XmlWriter& xml, const ExportOrder& exportOrder)
{
    xml.writeTag("framesettype");

    xml.writeTagAttribute("name", exportOrder.name());

    for (const auto& sf : exportOrder.stillFrames()) {
        writeExportName(xml, "frame", sf.first, sf.second);
    }

    for (const auto& ani : exportOrder.animations()) {
        writeExportName(xml, "animation", ani.first, ani.second);
    }

    xml.writeCloseTag();
}
}
}

/*
 * API
 * ===
 */

void readFile(ExportOrder& exportOrder, const std::string& filename)
{
    auto xml = XmlReader::fromFile(filename);
    std::unique_ptr<XmlTag> tag = xml->parseTag();

    if (tag->name != "framesettype") {
        throw std::runtime_error(filename + ": Not frame set type file");
    }

    Serializer::ExportOrderReader reader(exportOrder, *xml);
    reader.readExportOrder(tag.get());
}

void writeFile(const ExportOrder& exportOrder, const std::string& filename)
{
    UnTech::AtomicOfStream file(filename);

    XmlWriter xml(file, filename, "untech");

    Serializer::ExportOrderWriter::writeExportOrder(xml, exportOrder);

    file.commit();
}
}
}
}
