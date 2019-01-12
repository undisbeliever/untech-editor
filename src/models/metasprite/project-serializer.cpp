/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "project.h"

#include "framesetfile-serializer.h"
#include "metasprite-serializer.h"
#include "spriteimporter-serializer.h"
#include "models/common/atomicofstream.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"
#include "utsi2utms/utsi2utms.h"

using namespace UnTech;
using namespace UnTech::Xml;

namespace UnTech {
namespace MetaSprite {

const std::string Project::FILE_EXTENSION = "utmspro";

inline std::unique_ptr<Project> readProject(XmlReader& xml, const XmlTag* tag)
{
    if (tag == nullptr || tag->name != "metaspriteproject") {
        throw xml_error(xml, "Not a MetaSprite Project (expected <metaspriteproject>");
    }

    auto project = std::make_unique<Project>();

    std::unique_ptr<XmlTag> childTag;

    while ((childTag = xml.parseTag())) {
        if (childTag->name == "exportorder") {
            project->exportOrders.insert_back(childTag->getAttributeFilename("src"));
        }
        else if (childTag->name == "frameset") {
            readFrameSetFile(childTag.get(), project->frameSets);
        }
        else {
            throw unknown_tag_error(*childTag);
        }

        xml.parseCloseTag();
    }

    return project;
}

inline void writeProject(XmlWriter& xml, const Project& project)
{
    xml.writeTag("metaspriteproject");

    for (const auto& it : project.exportOrders) {
        xml.writeTag("exportorder");
        xml.writeTagAttributeFilename("src", it.filename);
        xml.writeCloseTag();
    }

    writeFrameSetFiles(xml, project.frameSets);

    xml.writeCloseTag();
}

/*
 * API
 * ===
 */

std::unique_ptr<Project> loadProject(const std::string& filename)
{
    auto xml = XmlReader::fromFile(filename);
    try {
        std::unique_ptr<XmlTag> tag = xml->parseTag();
        return readProject(*xml, tag.get());
    }
    catch (const std::exception& ex) {
        throw xml_error(*xml, "Error loading FrameSetExportOrder file", ex);
    }
}

void saveProject(const Project& project, const std::string& filename)
{
    AtomicOfStream file(filename);
    XmlWriter xml(file, filename, "untech");
    writeProject(xml, project);
    file.commit();
}
}
}
