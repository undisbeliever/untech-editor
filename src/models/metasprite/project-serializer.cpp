/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
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
                fs.type = FST::UNKNOWN;
                fs.filename = childTag->getAttributeFilename("src");
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
    if (filename.empty()) {
        return;
    }

    auto xml = XmlReader::fromFile(filename);

    try {
        std::unique_ptr<XmlTag> tag = xml->parseTag();

        if (tag == nullptr) {
            throw std::runtime_error(filename + ": Unknown file format");
        }
        else if (tag->name == "metasprite") {
            msFrameSet = MetaSprite::readFrameSet(*xml, tag.get());
            type = FrameSetType::METASPRITE;
        }
        else if (tag->name == "spriteimporter") {
            siFrameSet = SpriteImporter::readFrameSet(*xml, tag.get());
            type = FrameSetType::SPRITE_IMPORTER;
        }
        else {
            throw xml_error(*tag, "Unknown file type");
        }
    }
    catch (const std::exception& ex) {
        throw xml_error(*xml, "Unable to load FrameSet", ex);
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
