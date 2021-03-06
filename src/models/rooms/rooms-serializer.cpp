/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "rooms-serializer.h"
#include "models/common/atomicofstream.h"
#include "models/common/externalfilelist.h"
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

const std::string RoomInput::FILE_EXTENSION = "utroom";

static const EnumMap<RoomEntranceOrientation> roomEntranceOrientationEnumMap = {
    { "down-right", RoomEntranceOrientation::DOWN_RIGHT },
    { "down-left", RoomEntranceOrientation::DOWN_LEFT },
    { "up-right", RoomEntranceOrientation::UP_RIGHT },
    { "up-left", RoomEntranceOrientation::UP_LEFT },
};

static void readRoomEntrance(const XmlTag& tag, NamedList<RoomEntrance>& entrances)
{
    assert(tag.name == "entrance");

    entrances.insert_back();
    RoomEntrance& en = entrances.back();

    en.name = tag.getAttributeOptionalId("name");
    en.position = tag.getAttributeUpoint();
    en.orientation = tag.getAttributeEnum("orientation", roomEntranceOrientationEnumMap);
}

static void writeRoomEntrance(XmlWriter& xml, const RoomEntrance& entrance)
{
    xml.writeTag("entrance");
    xml.writeTagAttribute("name", entrance.name);
    xml.writeTagAttributeUpoint(entrance.position);
    xml.writeTagAttributeEnum("orientation", entrance.orientation, roomEntranceOrientationEnumMap);
    xml.writeCloseTag();
}

static void readEntityGroup(XmlReader& xml, const XmlTag& tag, NamedList<EntityGroup>& entityGroups)
{
    assert(tag.name == "entity-group");

    entityGroups.insert_back();
    auto& eg = entityGroups.back();

    eg.name = tag.getAttributeOptionalId("name");

    while (const auto childTag = xml.parseTag()) {
        if (childTag.name == "entity") {
            auto& e = eg.entities.emplace_back();
            e.name = childTag.getAttributeOptionalId("name");
            e.entityId = childTag.getAttributeOptionalId("entity");
            e.position = childTag.getAttributePoint();
            e.parameter = childTag.getAttributeOrEmpty("parameter");
        }
        else {
            throw unknown_tag_error(childTag);
        }
        xml.parseCloseTag();
    }
}

static void writeEntityGroup(XmlWriter& xml, const EntityGroup& entityGroup)
{
    xml.writeTag("entity-group");
    xml.writeTagAttribute("name", entityGroup.name);

    for (const auto& e : entityGroup.entities) {
        xml.writeTag("entity");

        xml.writeTagAttributeOptional("name", e.name);
        xml.writeTagAttribute("entity", e.entityId);
        xml.writeTagAttributePoint(e.position);
        xml.writeTagAttribute("parameter", e.parameter);

        xml.writeCloseTag();
    }

    xml.writeCloseTag();
}

static void readScriptTrigger(const XmlTag& tag, std::vector<ScriptTrigger>& scriptTriggers)
{
    assert(tag.name == "script-trigger");

    ScriptTrigger& st = scriptTriggers.emplace_back();

    st.script = tag.getAttributeOptionalId("script");
    st.aabb = tag.getAttributeUrect();
    st.once = tag.getAttributeBoolean("once");
}

static void writeScriptTrigger(XmlWriter& xml, const ScriptTrigger& st)
{
    xml.writeTag("script-trigger");
    xml.writeTagAttribute("script", st.script);
    xml.writeTagAttributeUrect(st.aabb);
    xml.writeTagAttribute("once", st.once);
    xml.writeCloseTag();
}

static std::unique_ptr<RoomInput> readRoomInput(XmlReader& xml, const XmlTag& tag)
{
    if (tag.name != "room") {
        throw xml_error(xml, "Not a Room file (expected <room> tag)");
    }

    auto roomInput = std::make_unique<RoomInput>();

    roomInput->name = tag.getAttributeOptionalId("name");
    roomInput->scene = tag.getAttributeOptionalId("scene");

    while (const auto childTag = xml.parseTag()) {
        if (childTag.name == "map") {
            roomInput->map = MetaTiles::readMetaTileGrid(xml, childTag);
        }
        else if (childTag.name == "entrance") {
            readRoomEntrance(childTag, roomInput->entrances);
        }
        else if (childTag.name == "entity-group") {
            readEntityGroup(xml, childTag, roomInput->entityGroups);
        }
        else if (childTag.name == "script") {
            Scripting::readScript(roomInput->roomScripts, xml, childTag);
        }
        else if (childTag.name == "temp-script-variables") {
            Scripting::readTempScriptVariables(roomInput->roomScripts, xml, childTag);
        }
        else if (childTag.name == "script-trigger") {
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
    xml.writeTag("room");

    xml.writeTagAttributeOptional("name", input.name);
    xml.writeTagAttributeOptional("scene", input.scene);

    MetaTiles::writeMetaTileGrid(xml, "map", input.map);

    for (auto& en : input.entrances) {
        writeRoomEntrance(xml, en);
    }
    for (auto& eg : input.entityGroups) {
        writeEntityGroup(xml, eg);
    }

    Scripting::writeRoomScripts(xml, input.roomScripts);

    for (auto& st : input.scriptTriggers) {
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
        throw xml_error(xml, "Error loading room", ex);
    }
}

std::unique_ptr<RoomInput> loadRoomInput(const std::filesystem::path& filename)
{
    auto xml = XmlReader::fromFile(filename);
    return readRoomInput(*xml);
}

void saveRoomInput(const RoomInput& input, const std::filesystem::path& filename)
{
    AtomicOfStream file(filename);
    XmlWriter xml(file, filename, "untech");
    writeRoomInput(xml, input);
    file.commit();
}

void readRoomSettings(RoomSettings& settings, const XmlTag& tag)
{
    settings.roomDataSize = tag.getAttributeUnsigned("room-data-size");
}

void writeRoomSettings(XmlWriter& xml, const RoomSettings& settings)
{
    xml.writeTag("room-settings");
    xml.writeTagAttribute("room-data-size", settings.roomDataSize);
    xml.writeCloseTag();
}

}
