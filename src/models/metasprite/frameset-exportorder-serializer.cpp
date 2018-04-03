/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "frameset-exportorder.h"

#include "models/common/atomicofstream.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"
#include <cassert>
#include <stdexcept>

using namespace UnTech;
using namespace UnTech::Xml;

namespace MS = UnTech::MetaSprite;

const std::string MS::FrameSetExportOrder::FILE_EXTENSION = "utfseo";

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
        assert(tag->name == "fsexportorder");
        assert(exportOrder.stillFrames.size() == 0);
        assert(exportOrder.animations.size() == 0);

        exportOrder.name = tag->getAttributeId("name");

        std::unique_ptr<XmlTag> childTag;

        while ((childTag = xml.parseTag())) {
            if (childTag->name == "frame") {
                readExportName(childTag.get(), exportOrder.stillFrames);
            }
            else if (childTag->name == "animation") {
                readExportName(childTag.get(), exportOrder.animations);
            }
            else {
                throw unknown_tag_error(*childTag);
            }

            xml.parseCloseTag();
        }
    }

private:
    inline void readExportName(const XmlTag* tag,
                               FrameSetExportOrder::ExportName::list_t& exportList)
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

                en.alternatives.emplace_back(alt);
            }
            else {
                throw unknown_tag_error(*childTag);
            }

            xml.parseCloseTag();
        }

        exportList.push_back(en);
    }
};

static void writeExportName(XmlWriter& xml, const std::string& tagName,
                            const FrameSetExportOrder::ExportName& exportName)
{
    xml.writeTag(tagName);

    xml.writeTagAttribute("id", exportName.name);

    for (const auto& alt : exportName.alternatives) {
        xml.writeTag("alt");
        xml.writeTagAttribute("name", alt.name);
        xml.writeTagAttribute("hflip", alt.hFlip);
        xml.writeTagAttribute("vflip", alt.vFlip);
        xml.writeCloseTag();
    }

    xml.writeCloseTag();
}

static void writeFrameSetExportOrder(XmlWriter& xml, const FrameSetExportOrder& eo)
{
    xml.writeTag("fsexportorder");

    xml.writeTagAttribute("name", eo.name);

    for (const auto& frame : eo.stillFrames) {
        writeExportName(xml, "frame", frame);
    }
    for (const auto& ani : eo.animations) {
        writeExportName(xml, "animation", ani);
    }

    xml.writeCloseTag();
}

std::unique_ptr<FrameSetExportOrder> loadFrameSetExportOrder(const std::string& filename)
{
    auto xml = XmlReader::fromFile(filename);
    try {
        std::unique_ptr<XmlTag> tag = xml->parseTag();

        if (tag == nullptr || tag->name != "fsexportorder") {
            throw std::runtime_error(filename + ": Not frame set export order file");
        }

        auto exportOrder = std::make_unique<FrameSetExportOrder>();
        FrameSetExportOrderReader reader(*exportOrder, *xml);
        reader.readFrameSetExportOrder(tag.get());

        return exportOrder;
    }
    catch (const std::exception& ex) {
        throw xml_error(*xml, "Error loading FrameSetExportOrder file", ex);
    }
}

void saveFrameSetExportOrder(const FrameSetExportOrder& eo, const std::string& filename)
{
    AtomicOfStream file(filename);
    XmlWriter xml(file, filename, "untech");
    writeFrameSetExportOrder(xml, eo);
    file.commit();
}
}
}
