/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "rooms-serializer.h"
#include "models/common/externalfilelist.h"
#include "models/common/file.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"
#include "models/metatiles/metatiles-serializer.h"
#include "models/scripting/scripting-serializer.h"

using namespace UnTech::Xml;

namespace UnTech {
template <>
void UnTech::ExternalFileItem<UnTech::Rooms::RoomInput>::loadFile()
{
    value = Rooms::loadRoomInput(filename);
}
}

namespace UnTech::Rooms {

const std::u8string RoomInput::FILE_EXTENSION = u8"utroom";

static const EnumMap<RoomEntranceOrientation> roomEntranceOrientationEnumMap = {
    { u8"down-right", RoomEntranceOrientation::DOWN_RIGHT },
    { u8"down-left", RoomEntranceOrientation::DOWN_LEFT },
    { u8"up-right", RoomEntranceOrientation::UP_RIGHT },
    { u8"up-left", RoomEntranceOrientation::UP_LEFT },
};

static void readRoomEntrance(const XmlTag& tag, NamedList<RoomEntrance>& entrances)
{
    assert(tag.name == u8"entrance");

    entrances.insert_back();
    RoomEntrance& en = entrances.back();

    en.name = tag.getAttributeOptionalId(u8"name");
    en.position = tag.getAttributeUpoint();
    en.orientation = tag.getAttributeEnum(u8"orientation", roomEntranceOrientationEnumMap);
}

static void writeRoomEntrance(XmlWriter& xml, const RoomEntrance& entrance)
{
    xml.writeTag(u8"entrance");
    xml.writeTagAttribute(u8"name", entrance.name);
    xml.writeTagAttributeUpoint(entrance.position);
    xml.writeTagAttributeEnum(u8"orientation", entrance.orientation, roomEntranceOrientationEnumMap);
    xml.writeCloseTag();
}

static void readEntityGroup(XmlReader& xml, const XmlTag& tag, NamedList<EntityGroup>& entityGroups)
{
    assert(tag.name == u8"entity-group");

    entityGroups.insert_back();
    auto& eg = entityGroups.back();

    eg.name = tag.getAttributeOptionalId(u8"name");

    while (const auto childTag = xml.parseTag()) {
        if (childTag.name == u8"entity") {
            auto& e = eg.entities.emplace_back();
            e.name = childTag.getAttributeOptionalId(u8"name");
            e.entityId = childTag.getAttributeOptionalId(u8"entity");
            e.position = childTag.getAttributePoint();
            e.parameter = childTag.getAttributeOrEmpty(u8"parameter");
        }
        else {
            throw unknown_tag_error(childTag);
        }
        xml.parseCloseTag();
    }
}

static void writeEntityGroup(XmlWriter& xml, const EntityGroup& entityGroup)
{
    xml.writeTag(u8"entity-group");
    xml.writeTagAttribute(u8"name", entityGroup.name);

    for (const auto& e : entityGroup.entities) {
        xml.writeTag(u8"entity");

        xml.writeTagAttributeOptional(u8"name", e.name);
        xml.writeTagAttribute(u8"entity", e.entityId);
        xml.writeTagAttributePoint(e.position);
        xml.writeTagAttribute(u8"parameter", e.parameter);

        xml.writeCloseTag();
    }

    xml.writeCloseTag();
}

static void readScriptTrigger(const XmlTag& tag, std::vector<ScriptTrigger>& scriptTriggers)
{
    assert(tag.name == u8"script-trigger");

    ScriptTrigger& st = scriptTriggers.emplace_back();

    st.script = tag.getAttributeOptionalId(u8"script");
    st.aabb = tag.getAttributeUrect();
    st.once = tag.getAttributeBoolean(u8"once");
}

static void writeScriptTrigger(XmlWriter& xml, const ScriptTrigger& st)
{
    xml.writeTag(u8"script-trigger");
    xml.writeTagAttribute(u8"script", st.script);
    xml.writeTagAttributeUrect(st.aabb);
    xml.writeTagAttribute(u8"once", st.once);
    xml.writeCloseTag();
}

static std::unique_ptr<RoomInput> readRoomInput(XmlReader& xml, const XmlTag& tag)
{
    if (tag.name != u8"room") {
        throw xml_error(xml, u8"Not a Room file (expected <room> tag)");
    }

    auto roomInput = std::make_unique<RoomInput>();

    roomInput->name = tag.getAttributeOptionalId(u8"name");
    roomInput->scene = tag.getAttributeOptionalId(u8"scene");

    while (const auto childTag = xml.parseTag()) {
        if (childTag.name == u8"map") {
            roomInput->map = MetaTiles::readMetaTileGrid(xml, childTag);
        }
        else if (childTag.name == u8"entrance") {
            readRoomEntrance(childTag, roomInput->entrances);
        }
        else if (childTag.name == u8"entity-group") {
            readEntityGroup(xml, childTag, roomInput->entityGroups);
        }
        else if (childTag.name == u8"script") {
            Scripting::readScript(roomInput->roomScripts, xml, childTag);
        }
        else if (childTag.name == u8"temp-script-variables") {
            Scripting::readTempScriptVariables(roomInput->roomScripts, xml, childTag);
        }
        else if (childTag.name == u8"script-trigger") {
            readScriptTrigger(childTag, roomInput->scriptTriggers);
        }
        else {
            throw unknown_tag_error(childTag);
        }

        xml.parseCloseTag();
    }

    return roomInput;
}

void writeRoomInput(XmlWriter& xml, const RoomInput& input)
{
    xml.writeTag(u8"room");

    xml.writeTagAttributeOptional(u8"name", input.name);
    xml.writeTagAttributeOptional(u8"scene", input.scene);

    MetaTiles::writeMetaTileGrid(xml, u8"map", input.map);

    for (const auto& en : input.entrances) {
        writeRoomEntrance(xml, en);
    }
    for (const auto& eg : input.entityGroups) {
        writeEntityGroup(xml, eg);
    }

    Scripting::writeRoomScripts(xml, input.roomScripts);

    for (const auto& st : input.scriptTriggers) {
        writeScriptTrigger(xml, st);
    }

    xml.writeCloseTag();
}

std::unique_ptr<RoomInput> readRoomInput(Xml::XmlReader& xml)
{
    try {
        const auto tag = xml.parseTag();
        return readRoomInput(xml, tag);
    }
    catch (const std::exception& ex) {
        throw xml_error(xml, u8"Error loading room", ex);
    }
}

std::unique_ptr<RoomInput> loadRoomInput(const std::filesystem::path& filename)
{
    auto xml = XmlReader::fromFile(filename);
    return readRoomInput(*xml);
}

void saveRoomInput(const RoomInput& input, const std::filesystem::path& filename)
{
    // utroom files contain a large base64 text block, use a larger buffer.
    XmlWriter xml(filename, u8"untech", 128 * 1024);
    writeRoomInput(xml, input);

    File::atomicWrite(filename, xml.string_view());
}

void readRoomSettings(RoomSettings& settings, const XmlTag& tag)
{
    settings.roomDataSize = tag.getAttributeUnsigned(u8"room-data-size");
}

void writeRoomSettings(XmlWriter& xml, const RoomSettings& settings)
{
    xml.writeTag(u8"room-settings");
    xml.writeTagAttribute(u8"room-data-size", settings.roomDataSize);
    xml.writeCloseTag();
}

}
