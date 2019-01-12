/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framesetfile-serializer.h"

using namespace UnTech;
using namespace UnTech::Xml;

namespace UnTech {
namespace MetaSprite {

const EnumMap<FrameSetFile::FrameSetType> frameSetTypeMap = {
    { "none", FrameSetFile::FrameSetType::NONE },
    { "unknown", FrameSetFile::FrameSetType::UNKNOWN },
    { "metasprite", FrameSetFile::FrameSetType::METASPRITE },
    { "spriteimporter", FrameSetFile::FrameSetType::SPRITE_IMPORTER },
};

void readFrameSetFile(const XmlTag* tag, std::vector<FrameSetFile>& frameSets)
{
    assert(tag->name == "frameset");

    frameSets.emplace_back();
    FrameSetFile& fs = frameSets.back();

    if (tag->hasAttribute("src")) {
        fs.filename = tag->getAttributeFilename("src");

        if (tag->hasAttribute("type")) {
            fs.type = tag->getAttributeEnum("type", frameSetTypeMap);
        }
        else {
            fs.setTypeFromExtension();
        }
    }
    else {
        fs.type = FrameSetFile::FrameSetType::NONE;
    }
}

void writeFrameSetFiles(XmlWriter& xml, const std::vector<FrameSetFile>& frameSets)
{
    using FST = FrameSetFile::FrameSetType;

    for (const auto& fs : frameSets) {
        xml.writeTag("frameset");
        if (fs.type != FST::NONE && !fs.filename.empty()) {
            xml.writeTagAttributeFilename("src", fs.filename);
            xml.writeTagAttributeEnum("type", fs.type, frameSetTypeMap);
        }
        xml.writeCloseTag();
    }
}

}
}
