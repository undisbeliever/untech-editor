/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "project.h"

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

const EnumMap<Project::FrameSetType> frameSetTypeMap = {
    { "none", Project::FrameSetType::NONE },
    { "unknown", Project::FrameSetType::UNKNOWN },
    { "metasprite", Project::FrameSetType::METASPRITE },
    { "spriteimporter", Project::FrameSetType::SPRITE_IMPORTER },
};

inline std::unique_ptr<Project> readProject(XmlReader& xml, const XmlTag* tag)
{
    using FST = Project::FrameSetType;

    if (tag == nullptr || tag->name != "metaspriteproject") {
        throw xml_error(xml, "Not a MetaSprite Project (expected <metaspriteproject>");
    }

    auto project = std::make_unique<Project>();

    std::unique_ptr<XmlTag> childTag;

    while ((childTag = xml.parseTag())) {
        if (childTag->name == "frameset") {
            project->frameSets.emplace_back();
            Project::FrameSetFile& fs = project->frameSets.back();

            if (childTag->hasAttribute("src")) {
                fs.filename = childTag->getAttributeFilename("src");

                if (childTag->hasAttribute("type")) {
                    fs.type = childTag->getAttributeEnum("type", frameSetTypeMap);
                }
                else {
                    fs.setTypeFromExtension();
                }
            }
            else {
                fs.type = FST::NONE;
            }
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
    using FST = Project::FrameSetType;

    xml.writeTag("metaspriteproject");

    for (const auto& fs : project.frameSets) {
        xml.writeTag("frameset");
        if (fs.type != FST::NONE && !fs.filename.empty()) {
            xml.writeTagAttributeFilename("src", fs.filename);
            xml.writeTagAttributeEnum("type", fs.type, frameSetTypeMap);
        }
        xml.writeCloseTag();
    }

    xml.writeCloseTag();
}

/*
 * API
 * ===
 */

void Project::FrameSetFile::loadFile()
{
    msFrameSet = nullptr;
    siFrameSet = nullptr;

    if (filename.empty()) {
        return;
    }

    switch (type) {
    case FrameSetType::NONE:
        break;

    case FrameSetType::METASPRITE:
        msFrameSet = MetaSprite::loadFrameSet(filename);
        break;

    case FrameSetType::SPRITE_IMPORTER:
        siFrameSet = SpriteImporter::loadFrameSet(filename);
        break;

    case FrameSetType::UNKNOWN:
        throw std::runtime_error("Cannot load " + filename + ": Unknown frameset type");
        break;
    }
}

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
