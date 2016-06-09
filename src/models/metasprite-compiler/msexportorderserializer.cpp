#include "msexportorderserializer.h"
#include "msexportorder.h"
#include "models/common/atomicofstream.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"
#include "models/metasprite/document.h"
#include <cassert>
#include <fstream>
#include <stdexcept>

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
        assert(tag->name == "metaspriteexport");
        assert(msExportOrder.frameSets().size() == 0);

        std::unique_ptr<XmlTag> childTag;

        while ((childTag = xml.parseTag())) {
            if (childTag->name == "frameset") {
                readMetaSpriteDocument(childTag.get());
            }
            else {
                throw childTag->buildUnknownTagError();
            }

            xml.parseCloseTag();
        }
    }

private:
    inline void readMetaSpriteDocument(const XmlTag* tag)
    {
        typedef UnTech::MetaSprite::MetaSpriteDocument MetaSpriteDocument;
        assert(tag->name == "frameset");

        if (tag->hasAttribute("src")) {
            const std::string src = tag->getAttributeFilename("src");

            try {
                msExportOrder.frameSets().emplace_back(
                    std::make_shared<MetaSpriteDocument>(src));
            }
            catch (const std::exception& ex) {
                throw tag->buildError("Unable to open MetaSprite document", ex);
            }
        }
        else {
            msExportOrder.frameSets().emplace_back(nullptr);
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
