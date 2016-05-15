#include "msexportorderserializer.h"
#include "msexportorder.h"
#include "models/metasprite/document.h"
#include "models/common/atomicofstream.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"
#include <cassert>
#include <stdexcept>
#include <fstream>

using namespace UnTech;
using namespace UnTech::Xml;

namespace UnTech {
namespace MetaSpriteCompiler {
namespace Serializer {

/*
 * EXPORT ORDER READER
 * ====================
 */

struct MsExportOrderReader {
    MsExportOrderReader(MsExportOrder& msExportOrder, XmlReader& xml)
        : msExportOrder(msExportOrder)
        , xml(xml)
    {
    }

private:
    MsExportOrder& msExportOrder;
    XmlReader& xml;

public:
    inline void readMsExportOrder(const XmlTag* tag)
    {
        typedef UnTech::MetaSprite::MetaSpriteDocument MetaSpriteDocument;

        assert(tag->name == "metaspriteexport");
        assert(msExportOrder.frameSets().size() == 0);

        std::unique_ptr<XmlTag> childTag;

        while ((childTag = xml.parseTag())) {
            if (childTag->name == "frameset") {
                if (childTag->hasAttribute("src")) {
                    const std::string src = childTag->getAttributeFilename("src");

                    msExportOrder.frameSets().emplace_back(
                        std::make_shared<MetaSpriteDocument>(src));
                }
                else {
                    msExportOrder.frameSets().emplace_back(nullptr);
                }
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

namespace MsExportOrderWriter {

inline void writeMsExportOrder(XmlWriter& xml, const MsExportOrder& msExportOrder)
{
    xml.writeTag("metaspriteexport");

    for (const auto& fs : msExportOrder.frameSets()) {
        xml.writeTag("frameset");

        if (fs) {
            xml.writeTagAttributeFilename("src", fs->filename());
        }

        xml.writeCloseTag();
    }

    xml.writeCloseTag();
}
}

/*
 * API
 * ===
 */

void readFile(MsExportOrder& msExportOrder, const std::string& filename)
{
    auto xml = XmlReader::fromFile(filename);
    std::unique_ptr<XmlTag> tag = xml->parseTag();

    if (tag->name != "metaspriteexport") {
        throw std::runtime_error(filename + ": Not a MetaSprite Exporter file");
    }

    Serializer::MsExportOrderReader reader(msExportOrder, *xml);
    reader.readMsExportOrder(tag.get());
}

void writeFile(const MsExportOrder& msExportOrder, const std::string& filename)
{
    UnTech::AtomicOfStream file(filename);

    XmlWriter xml(file, filename, "untech");

    Serializer::MsExportOrderWriter::writeMsExportOrder(xml, msExportOrder);

    file.commit();
}
}
}
}
