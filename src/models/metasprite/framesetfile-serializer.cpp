/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framesetfile-serializer.h"

using namespace UnTech::Xml;

namespace UnTech::MetaSprite {

const EnumMap<FrameSetFile::FrameSetType> frameSetTypeMap = {
    { u8"unknown", FrameSetFile::FrameSetType::UNKNOWN },
    { u8"metasprite", FrameSetFile::FrameSetType::METASPRITE },
    { u8"spriteimporter", FrameSetFile::FrameSetType::SPRITE_IMPORTER },
};

void readFrameSetFile(const XmlTag& tag, std::vector<FrameSetFile>& frameSets)
{
    assert(tag.name == u8"frameset");

    frameSets.emplace_back();
    FrameSetFile& fs = frameSets.back();

    fs.filename = tag.getAttributeFilename(u8"src");

    if (tag.hasAttribute(u8"type")) {
        fs.type = tag.getAttributeEnum(u8"type", frameSetTypeMap);
    }
    else {
        fs.setTypeFromExtension();
    }
}

void writeFrameSetFiles(XmlWriter& xml, const std::vector<FrameSetFile>& frameSets)
{
    for (const auto& fs : frameSets) {
        xml.writeTag(u8"frameset");
        xml.writeTagAttributeFilename(u8"src", fs.filename);
        xml.writeTagAttributeEnum(u8"type", fs.type, frameSetTypeMap);

        xml.writeCloseTag();
    }
}

}
