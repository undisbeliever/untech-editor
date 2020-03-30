/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "rooms-serializer.h"
#include "models/common/atomicofstream.h"
#include "models/common/externalfilelist.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"
#include "models/metatiles/metatiles-serializer.h"

using namespace UnTech;
using namespace UnTech::Xml;

namespace UnTech {
template <>
void UnTech::ExternalFileItem<::Rooms::RoomInput>::loadFile()
{
    value = Rooms::loadRoomInput(filename);
}
}

namespace UnTech::Rooms {

const std::string RoomInput::FILE_EXTENSION = "utroom";

static std::unique_ptr<RoomInput> readRoomInput(XmlReader& xml, const XmlTag* tag)
{
    if (tag == nullptr || tag->name != "room") {
        throw xml_error(xml, "Not a Room file (expected <room> tag)");
    }

    auto roomInput = std::make_unique<RoomInput>();

    roomInput->name = tag->getAttributeOptionalId("name");
    roomInput->scene = tag->getAttributeOptionalId("scene");

    while (auto childTag = xml.parseTag()) {
        if (childTag->name == "map") {
            roomInput->map = MetaTiles::readMetaTileGrid(xml, childTag.get());
        }
        else {
            throw unknown_tag_error(*childTag);
        }

        xml.parseCloseTag();
    }

    return roomInput;
}

static void writeRoomInput(XmlWriter& xml, const RoomInput& input)
{
    xml.writeTag("room");

    xml.writeTagAttributeOptional("name", input.name);
    xml.writeTagAttributeOptional("scene", input.scene);

    MetaTiles::writeMetaTileGrid(xml, "map", input.map);

    xml.writeCloseTag();
}

std::unique_ptr<RoomInput> loadRoomInput(const std::filesystem::path& filename)
{
    auto xml = XmlReader::fromFile(filename);
    try {
        std::unique_ptr<XmlTag> tag = xml->parseTag();
        return readRoomInput(*xml, tag.get());
    }
    catch (const std::exception& ex) {
        throw xml_error(*xml, "Error loading metatile tileset", ex);
    }
}

void saveRoomInput(const RoomInput& input, const std::filesystem::path& filename)
{
    AtomicOfStream file(filename);
    XmlWriter xml(file, filename, "untech");
    writeRoomInput(xml, input);
    file.commit();
}

}
