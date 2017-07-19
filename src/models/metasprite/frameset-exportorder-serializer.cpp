/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "frameset-exportorder.h"

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

        bool idExists = std::any_of(
            exportList.begin(), exportList.end(),
            [en](const auto& existing) { return existing.name == en.name; });

        if (idExists) {
            throw xml_error(*tag, "id already exists");
        }

        std::unique_ptr<XmlTag> childTag;

        while ((childTag = xml.parseTag())) {
            if (childTag->name == "alt") {
                NameReference alt;

                alt.name = childTag->getAttributeId("name");
                alt.hFlip = childTag->getAttributeBoolean("hflip");
                alt.vFlip = childTag->getAttributeBoolean("vflip");

                auto it = std::find(en.alternatives.begin(), en.alternatives.end(), alt);
                if (it != en.alternatives.end()) {
                    throw xml_error(*childTag, "alt already exists");
                }

                en.alternatives.push_back(alt);
            }
            else {
                throw unknown_tag_error(*childTag);
            }

            xml.parseCloseTag();
        }

        exportList.push_back(en);
    }
};

std::shared_ptr<const FrameSetExportOrder>
loadFrameSetExportOrderFile(const std::string& filename)
{
    auto xml = XmlReader::fromFile(filename);
    try {
        std::unique_ptr<XmlTag> tag = xml->parseTag();

        if (tag == nullptr || tag->name != "fsexportorder") {
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

std::shared_ptr<const FrameSetExportOrder>
loadFrameSetExportOrderCached(const std::string& filename)
{
    static std::unordered_map<std::string, std::weak_ptr<const FrameSetExportOrder>> cache;

    const std::string fullPath = File::fullPath(filename);
    auto& c = cache[fullPath];

    std::shared_ptr<const FrameSetExportOrder> eo = c.lock();
    if (eo) {
        return eo;
    }
    else {
        // not found
        eo = loadFrameSetExportOrderFile(fullPath);
        c = eo;
        return eo;
    }
}
}
}
