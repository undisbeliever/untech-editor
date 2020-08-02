/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "entityromdata-serializer.h"
#include <cassert>

namespace UnTech {
namespace Entity {

static const EnumMap<EntityType> entityTypeMap = {
    { "entity", EntityType::ENTITY },
    { "projectile", EntityType::PROJECTILE },
    { "player", EntityType::PLAYER },
};

static const EnumMap<DataType> dataTypeMap = {
    { "uint8", DataType::UINT8 },
    { "uint16", DataType::UINT16 },
    { "uint24", DataType::UINT24 },
    { "uint32", DataType::UINT32 },
    { "sint8", DataType::SINT8 },
    { "sint16", DataType::SINT16 },
    { "sint24", DataType::SINT24 },
    { "sint32", DataType::SINT32 },
};

static const EnumMap<ParameterType> parameterTypeMap = {
    { "unused", ParameterType::UNUSED },
    { "unsigned", ParameterType::UNSIGNED_BYTE },
};

using namespace Xml;

static void readEntityRomStruct(XmlReader& xml, const XmlTag* tag, NamedList<EntityRomStruct>& structs)
{
    assert(tag->name == "struct");

    structs.insert_back();
    EntityRomStruct& romStruct = structs.back();

    romStruct.name = tag->getAttributeOptionalId("name");
    romStruct.parent = tag->getAttributeOptionalId("parent");
    romStruct.comment = tag->getAttributeOrEmpty("comment");

    while (auto childTag = xml.parseTag()) {
        if (childTag->name == "struct-field") {
            romStruct.fields.emplace_back(
                StructField{
                    childTag->getAttributeOptionalId("name"),
                    childTag->getAttributeEnum("type", dataTypeMap),
                    childTag->getAttributeOrEmpty("default"),
                    childTag->getAttributeOrEmpty("comment") });
        }
        else {
            throw unknown_tag_error(*childTag);
        }

        xml.parseCloseTag();
    }
}

static void readEntityFunctionTable(const XmlTag* tag, NamedList<EntityFunctionTable>& functionTables)
{
    assert(tag->name == "function-table");

    functionTables.insert_back();
    EntityFunctionTable& ft = functionTables.back();

    ft.name = tag->getAttributeOptionalId("name");
    if (tag->hasAttribute("type")) {
        ft.entityType = tag->getAttributeEnum("type", entityTypeMap);
    }
    ft.entityStruct = tag->getAttributeOptionalId("struct");
    ft.exportOrder = tag->getAttributeOptionalId("export-order");
    ft.parameterType = tag->getAttributeEnum("parameter-type", parameterTypeMap);
    ft.comment = tag->getAttributeOrEmpty("comment");
}

static void readEntityRomEntry(XmlReader& xml, const XmlTag* tag, const EntityType entityType,
                               NamedList<EntityRomEntry>& entries)
{
    assert(tag->name == "entry");

    entries.insert_back();
    EntityRomEntry& entry = entries.back();

    entry.name = tag->getAttributeOptionalId("name");
    entry.functionTable = tag->getAttributeOptionalId("function-table");
    entry.comment = tag->getAttributeOrEmpty("comment");
    entry.initialProjectileId = tag->getAttributeOptionalId("projectileid");
    if (entityType != EntityType::PLAYER) {
        entry.initialListId = tag->getAttributeOptionalId("listid");
    }
    entry.frameSetId = tag->getAttributeOptionalId("frameset");
    entry.displayFrame = tag->getAttributeOptionalId("frame");
    entry.defaultPalette = tag->getAttributeUnsigned("palette");

    while (auto childTag = xml.parseTag()) {
        if (childTag->name == "entry-field") {
            idstring fieldFor = childTag->getAttributeId("for");
            std::string fieldValue = childTag->getAttribute("value");

            if (entry.fields.find(fieldFor) != entry.fields.end()) {
                throw xml_error(*childTag, "Duplicate entry-field detected");
            }
            entry.fields.emplace(fieldFor, fieldValue);
        }
        else {
            throw unknown_tag_error(*childTag);
        }

        xml.parseCloseTag();
    }
}

static void readEntityRomEntries(XmlReader& xml, const EntityType entityType,
                                 NamedList<EntityRomEntry>& entries)
{
    while (auto childTag = xml.parseTag()) {
        if (childTag->name == "entry") {
            readEntityRomEntry(xml, childTag.get(), entityType, entries);
        }
        else {
            throw unknown_tag_error(*childTag);
        }

        xml.parseCloseTag();
    }
}

void readEntityRomData(XmlReader& xml, const XmlTag* tag, EntityRomData& entityRomData)
{
    assert(tag->name == "entity-rom-data");

    while (auto childTag = xml.parseTag()) {
        if (childTag->name == "listid") {
            entityRomData.listIds.push_back(childTag->getAttributeOptionalId("name"));
        }
        else if (childTag->name == "struct") {
            readEntityRomStruct(xml, childTag.get(), entityRomData.structs);
        }
        else if (childTag->name == "function-table") {
            readEntityFunctionTable(childTag.get(), entityRomData.functionTables);
        }
        else if (childTag->name == "entities") {
            readEntityRomEntries(xml, EntityType::ENTITY, entityRomData.entities);
        }
        else if (childTag->name == "projectiles") {
            readEntityRomEntries(xml, EntityType::PROJECTILE, entityRomData.projectiles);
        }
        else if (childTag->name == "players") {
            readEntityRomEntries(xml, EntityType::PLAYER, entityRomData.players);
        }
        else {
            throw unknown_tag_error(*childTag);
        }

        xml.parseCloseTag();
    }
}

void writeEntityRomEntries(XmlWriter& xml, const std::string& tagName, const EntityType entityType,
                           const NamedList<EntityRomEntry>& entries)
{
    if (entries.empty()) {
        return;
    }

    xml.writeTag(tagName);

    for (auto& entry : entries) {
        xml.writeTag("entry");

        xml.writeTagAttributeOptional("name", entry.name);
        xml.writeTagAttributeOptional("function-table", entry.functionTable);
        xml.writeTagAttributeOptional("comment", entry.comment);
        xml.writeTagAttributeOptional("projectileid", entry.initialProjectileId);
        if (entityType != EntityType::PLAYER) {
            xml.writeTagAttributeOptional("listid", entry.initialListId);
        }
        xml.writeTagAttributeOptional("frameset", entry.frameSetId);
        xml.writeTagAttributeOptional("frame", entry.displayFrame);
        xml.writeTagAttribute("palette", entry.defaultPalette);

        // Sort fields so output is the same on all systems
        using FieldValue = std::unordered_map<idstring, std::string>::value_type;
        std::vector<std::reference_wrapper<const FieldValue>> fields(entry.fields.begin(), entry.fields.end());
        std::sort(fields.begin(), fields.end(),
                  [](const FieldValue& a, const FieldValue& b) { return a.first < b.first; });

        for (const auto& f : fields) {
            if (!f.get().second.empty()) {
                xml.writeTag("entry-field");
                xml.writeTagAttribute("for", f.get().first);
                xml.writeTagAttribute("value", f.get().second);
                xml.writeCloseTag();
            }
        }

        xml.writeCloseTag();
    }

    xml.writeCloseTag();
}

void writeEntityRomData(XmlWriter& xml, const EntityRomData& entityRomData)
{
    xml.writeTag("entity-rom-data");

    for (auto& listId : entityRomData.listIds) {
        xml.writeTag("listid");
        xml.writeTagAttribute("name", listId);
        xml.writeCloseTag();
    }

    for (const auto& romStruct : entityRomData.structs) {
        xml.writeTag("struct");
        xml.writeTagAttributeOptional("name", romStruct.name);
        xml.writeTagAttributeOptional("parent", romStruct.parent);
        xml.writeTagAttributeOptional("comment", romStruct.comment);

        for (const auto& field : romStruct.fields) {
            xml.writeTag("struct-field");
            xml.writeTagAttributeOptional("name", field.name);
            xml.writeTagAttributeEnum("type", field.type, dataTypeMap);
            xml.writeTagAttributeOptional("default", field.defaultValue);
            xml.writeTagAttributeOptional("comment", field.comment);
            xml.writeCloseTag();
        }
        xml.writeCloseTag();
    }

    for (const auto& ft : entityRomData.functionTables) {
        xml.writeTag("function-table");
        xml.writeTagAttributeOptional("name", ft.name);
        xml.writeTagAttributeEnum("type", ft.entityType, entityTypeMap);
        xml.writeTagAttributeOptional("struct", ft.entityStruct);
        xml.writeTagAttributeOptional("export-order", ft.exportOrder);
        xml.writeTagAttributeEnum("parameter-type", ft.parameterType, parameterTypeMap);
        xml.writeTagAttributeOptional("comment", ft.comment);
        xml.writeCloseTag();
    }

    writeEntityRomEntries(xml, "entities", EntityType::ENTITY, entityRomData.entities);
    writeEntityRomEntries(xml, "projectiles", EntityType::PROJECTILE, entityRomData.projectiles);
    writeEntityRomEntries(xml, "players", EntityType::PLAYER, entityRomData.players);

    xml.writeCloseTag();
}

}
}
