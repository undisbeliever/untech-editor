/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "frameset-exportorder.h"

#include "models/common/exceptions.h"
#include "models/common/externalfilelist.h"
#include "models/common/file.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"
#include <cassert>

using namespace UnTech::Xml;

namespace UnTech {

template <>
void ExternalFileItem<MetaSprite::FrameSetExportOrder>::loadFile()
{
    value = MetaSprite::loadFrameSetExportOrder(filename);
}

}

namespace UnTech::MetaSprite {

const std::u8string FrameSetExportOrder::FILE_EXTENSION = u8"utfseo";

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
    inline void readFrameSetExportOrder(const XmlTag& tag)
    {
        assert(tag.name() == u8"fsexportorder");
        assert(exportOrder.stillFrames.size() == 0);
        assert(exportOrder.animations.size() == 0);

        exportOrder.name = tag.getAttributeId(u8"name");

        while (const auto childTag = xml.parseTag()) {
            if (childTag.name() == u8"frame") {
                readExportName(childTag, exportOrder.stillFrames);
            }
            else if (childTag.name() == u8"animation") {
                readExportName(childTag, exportOrder.animations);
            }
            else {
                throw unknown_tag_error(childTag);
            }

            xml.parseCloseTag();
        }
    }

private:
    inline void readExportName(const XmlTag& tag,
                               NamedList<FrameSetExportOrder::ExportName>& exportList)
    {
        FrameSetExportOrder::ExportName en;
        en.name = tag.getAttributeId(u8"id");

        while (const auto childTag = xml.parseTag()) {
            if (childTag.name() == u8"alt") {
                NameReference alt;

                alt.name = childTag.getAttributeId(u8"name");
                alt.hFlip = childTag.getAttributeBoolean(u8"hflip");
                alt.vFlip = childTag.getAttributeBoolean(u8"vflip");

                en.alternatives.emplace_back(alt);
            }
            else {
                throw unknown_tag_error(childTag);
            }

            xml.parseCloseTag();
        }

        exportList.insert_back(en);
    }
};

static void writeExportName(XmlWriter& xml, const std::u8string& tagName,
                            const FrameSetExportOrder::ExportName& exportName)
{
    xml.writeTag(tagName);

    xml.writeTagAttribute(u8"id", exportName.name);

    for (const auto& alt : exportName.alternatives) {
        xml.writeTag(u8"alt");
        xml.writeTagAttribute(u8"name", alt.name);
        xml.writeTagAttribute(u8"hflip", alt.hFlip);
        xml.writeTagAttribute(u8"vflip", alt.vFlip);
        xml.writeCloseTag();
    }

    xml.writeCloseTag();
}

void writeFrameSetExportOrder(XmlWriter& xml, const FrameSetExportOrder& eo)
{
    xml.writeTag(u8"fsexportorder");

    xml.writeTagAttribute(u8"name", eo.name);

    for (const auto& frame : eo.stillFrames) {
        writeExportName(xml, u8"frame", frame);
    }
    for (const auto& ani : eo.animations) {
        writeExportName(xml, u8"animation", ani);
    }

    xml.writeCloseTag();
}

std::unique_ptr<FrameSetExportOrder> readFrameSetExportOrder(Xml::XmlReader& xml)
{
    try {
        const auto tag = xml.parseTag();

        if (tag.name() != u8"fsexportorder") {
            throw runtime_error(xml.filename(), u8": Not frame set export order file");
        }

        auto exportOrder = std::make_unique<FrameSetExportOrder>();
        FrameSetExportOrderReader reader(*exportOrder, xml);
        reader.readFrameSetExportOrder(tag);

        return exportOrder;
    }
    catch (const std::exception& ex) {
        throw xml_error(xml, u8"Error loading FrameSetExportOrder file", ex);
    }
}

std::unique_ptr<FrameSetExportOrder> loadFrameSetExportOrder(const std::filesystem::path& filename)
{
    auto xml = XmlReader::fromFile(filename);
    return readFrameSetExportOrder(*xml);
}

void saveFrameSetExportOrder(const FrameSetExportOrder& eo, const std::filesystem::path& filename)
{
    XmlWriter xml(filename, u8"untech");
    writeFrameSetExportOrder(xml, eo);

    File::atomicWrite(filename, xml.string_view());
}

}
