/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framesetfile-serializer.h"

using namespace UnTech::Xml;

namespace UnTech::MetaSprite {

const EnumMap<FrameSetFile::FrameSetType> frameSetTypeMap = {
    { "unknown", FrameSetFile::FrameSetType::UNKNOWN },
    { "metasprite", FrameSetFile::FrameSetType::METASPRITE },
    { "spriteimporter", FrameSetFile::FrameSetType::SPRITE_IMPORTER },
};

void readFrameSetFile(const XmlTag& tag, std::vector<FrameSetFile>& frameSets)
{
    assert(tag.name == "frameset");

    frameSets.emplace_back();
    FrameSetFile& fs = frameSets.back();

    fs.filename = tag.getAttributeFilename("src");

    if (tag.hasAttribute("type")) {
        fs.type = tag.getAttributeEnum("type", frameSetTypeMap);
    }
    else {
        fs.setTypeFromExtension();
    }
}

void writeFrameSetFiles(XmlWriter& xml, const std::vector<FrameSetFile>& frameSets)
{
    for (const auto& fs : frameSets) {
        xml.writeTag("frameset");
        xml.writeTagAttributeFilename("src", fs.filename);
        xml.writeTagAttributeEnum("type", fs.type, frameSetTypeMap);

        xml.writeCloseTag();
    }
}

}
